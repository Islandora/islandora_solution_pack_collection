<?php

/**
 * @file
 * Miscellaneous helper functions.
 */

/**
 * Converts given model descriptions into Drupal form select element #options.
 *
 * @see islandora_get_content_models()
 *
 * @param array $content_models
 *   A list of content models as returned by islandora_get_content_models().
 *
 * @return array
 *   An associative array of content models for use in Drupal form select
 *   element options:
 *   - pid: The PID of the content model
 *     - label: The label to display to the user.
 */
function islandora_basic_collection_get_content_models_as_form_options(array $content_models) {
  $map_content_models_to_options = function($o) {
    $label = isset($o['label']) ? $o['label'] : $o['name'];
    return "{$o['pid']} ~ {$label}";
  };
  return array_map($map_content_models_to_options, $content_models);
}

/**
 * Lists collections for use in a Drupal form select element #options.
 *
 * Ignores the given collection.
 *
 * @see islandora_get_collections()
 *
 * @param FedoraObject $object
 *   A collection object to ignore.
 *
 * @return array
 *   An associative array of collections for use in Drupal form select
 *   element options:
 *   - pid: The PID of the collection object.
 *     - label: The label to display to the user.
 */
function islandora_basic_collection_get_other_collections_as_form_options(FedoraObject $object) {
  $map_options = function($o) {
    return $o['label'];
  };
  $options = array_map($map_options, islandora_basic_collection_get_collections());
  // Ignore given collection.
  unset($options[$object->id]);
  return $options;
}

/**
 * Gets a choose namespace element for use in the collection management forms.
 *
 * @param string $default_value
 *   The default value to use for the namespace field.
 *
 * @return array
 *   A Drupal form element for selecting the a namespace.
 */
function islandora_basic_collection_get_namespace_form_element($default_value) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $restrict_namespace = variable_get('islandora_namespace_restriction_enforced', FALSE);
  return array(
    '#type' => $restrict_namespace ? 'select' : 'textfield',
    '#title' => 'Collection Namespace',
    '#description' => t('Namespace for objects in this collection.'),
    '#size' => 15,
    '#options' => $restrict_namespace ? drupal_map_assoc(islandora_get_allowed_namespaces()) : NULL,
    '#default_value' => $default_value,
  );
}

/**
 * A Drupal form select table populated with the given collection's children.
 *
 * @param FedoraObject $object
 *   The collection object.
 * @param array $pager_options
 *   An associative array containing options to pass onto the pager.
 *   - element: The numeric ID of the pager
 *   - fragment: The url fragment to append onto pager links, includes '#'.
 *
 * @return array
 *   A Drupal select table form element.
 */
function islandora_basic_collection_get_children_select_table_form_element(FedoraObject $object, array $pager_options) {
  // Assumes all results are returned although the function description
  // states otherwise.
  $results = islandora_basic_collection_get_objects($object);
  $limit = 10;
  $page = pager_default_initialize(count($results), $limit, $pager_options['element']);
  $results = array_slice($results, $page * $limit, $limit);
  $rows = array();
  foreach ($results as $result) {
    $pid = $result['object']['value'];
    $label = empty($result['title']['value']) ? t('Untitled') : $result['title']['value'];
    $rows[$pid] = array(
      'label' => array(
        'data' => array(
          '#type' => 'link',
          '#title' => $label,
          '#href' => "islandora/object/{$pid}")));
  }
  // Theme pager doesn't support url fragments in D7 so we insert manually.
  $pager = theme('pager', array('quantity' => 10, 'element' => $pager_options['element']));
  $pattern = '/href="([^"]+)"/';
  $replace = 'href="\1' . $pager_options['fragment'] . '"';
  $pager = preg_replace($pattern, $replace, $pager);
  return array(
    '#type' => 'tableselect',
    '#header' => array('label' => array('data' => t('Label'))),
    '#options' => $rows,
    '#empty' => t('Collection is Empty.'),
    '#prefix' => $pager,
    '#suffix' => $pager,
  );
}

/**
 * Gets a list of all immediate child PIDs.
 *
 * @param FedoraObject $object
 *   The collection object.
 * @param string $type
 *   Limit the child PID's to those of the given content model.
 *
 * @return array
 *   The list of child PIDs.
 */
function islandora_basic_collection_get_child_pids(FedoraObject $object, $type = NULL) {
  $results = islandora_basic_collection_get_objects($object);
  if (isset($type)) {
    $filter_type = function($o) use($type) {
      return $o['content']['value'] == $type;
    };
    $results = array_filter($results, $filter_type);
  }
  $map_results = function($o) {
    return $o['object']['value'];
  };
  return array_map($map_results, $results);
}

/**
 * Gets a list of all parent pids.
 *
 * @param FedoraObject $object
 *   The object.
 *
 * @return array
 *   The list of parent PIDs.
 */
function islandora_basic_collection_get_parent_pids(FedoraObject $object) {
  $results = array_merge(
      $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection'),
      $object->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOf'));
  $map_results = function($o) {
    return $o['object']['value'];
  };
  $collections = array_map($map_results, $results);
  return array_unique(array_filter($collections));
}

/**
 * Gets a list of all parent pids exluding the given parent.
 *
 * @param FedoraObject $object
 *   The object.
 *
 * @return array
 *   The list of parent PIDs.
 */
function islandora_basic_collection_get_other_parents(FedoraObject $object, FedoraObject $parent) {
  $parents = islandora_basic_collection_get_parent_pids($object);
  unset($parents[array_search($parent->id, $parents)]);
  return $parents;
}

/**
 * Adds the given object to the given collection.
 *
 * @param FedoraObject $new_member
 *   The object to add.
 * @param FedoraObject $collection
 *   The collection object to add to.
 */
function islandora_basic_collection_add_to_collection(FedoraObject $new_member, FedoraObject $collection) {
  $results = $new_member->relationships->get(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $collection->id);
  if (empty($results)) {
    $new_member->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $collection->id);
  }
}

/**
 * Removes the given object from the given collection.
 *
 * @param FedoraObject $member
 *   The object to remove.
 * @param FedoraObject $collection
 *   The collection object to remove from.
 */
function islandora_basic_collection_remove_from_collection(FedoraObject $member, FedoraObject $collection) {
  $member->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $collection->id);
  $member->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOf', $collection->id);
}
