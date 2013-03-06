<?php

/**
 * @file
 * Handles the management of collection objects and their children.
 */

require_once dirname(__FILE__) . '/utilities.inc';
require_once dirname(__FILE__) . '/batch.inc';

/**
 * Manage Collection local task.
 *
 * Defines the actions to appear in the collection section of the Manage tab.
 *
 * @param FedoraObject $object
 *   The collection object.
 *
 * @return array
 *   A renderable array.
 */
function islandora_basic_collection_manage_object(FedoraObject $object) {
  drupal_set_title(t('Manage @label Collection', array('@label' => $object->label, '@pid' => $object->id)));
  // Defines accessible elements.
  $access = array(
    'create_child_collection' => islandora_basic_collection_ingest_child_collection_access($object),
    'policy_management' => user_access(MANAGE_COLLECTION_POLICY),
    'change_content_models' => user_access(MANAGE_CONTENT_MODEL_ASSOCIATION),
    'migrate_children' => user_access(MIGRATE_COLLECTION_MEMBERS),
    'delete_children' => user_access(FEDORA_PURGE),
  );
  // Build forms for only the accessible elements.
  foreach ($access as $element => $accessible) {
    $form = "islandora_basic_collection_{$element}_form";
    $forms[$element] = $accessible ? drupal_get_form($form, $object) : NULL;
  }
  // Determine the default element to display.
  return array(
    'manage_collection_object' => array(
      '#type' => 'vertical_tabs',
      'create_child_collection' => array(
        '#id' => 'create-child-collection',
        '#group' => 'manage_collection_object',
        '#access' => $access['create_child_collection'],
        '#title' => t('Create child collection'),
        '#type' => 'fieldset',
        'form' => $forms['create_child_collection'],
      ),
      'policy_management' => array(
        '#id' => 'policy-management',
        '#group' => 'manage_collection_object',
        '#access' => $access['policy_management'],
        '#type' => 'fieldset',
        '#title' => t('Manage collection policy'),
        'form' => $forms['policy_management'],
      ),
      'change_content_models' => array(
        '#id' => 'change-content-models',
        '#group' => 'manage_collection_object',
        '#access' => $access['change_content_models'],
        '#type' => 'fieldset',
        '#title' => t('Change content models'),
        'form' => $forms['change_content_models'],
      ),
      'migrate_children' => array(
        '#id' => 'migrate-children',
        '#group' => 'manage_collection_object',
        '#access' => $access['migrate_children'],
        '#type' => 'fieldset',
        '#title' => t('Migrate members'),
        'form' => $forms['migrate_children'],
      ),
      'delete_children' => array(
        '#id' => 'delete-children',
        '#group' => 'manage_collection_object',
        '#access' => $access['delete_children'],
        '#type' => 'fieldset',
        '#title' => t('Delete members of this collection'),
        'form' => $forms['delete_children'],
      ),
    ),
  );
}

/**
 * Define child collection form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The collection to add the child collection to.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_create_child_collection_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $policy = new CollectionPolicy($object['COLLECTION_POLICY']->content);
  $policy_content_models = $policy->getContentModels();
  $content_models = islandora_get_content_models();
  $form_state['collection'] = $object;
  $form_state['content_models'] = $content_models;
  $form_state['default_namespace'] = islandora_get_namespace($policy_content_models['islandora:collectionCModel']['namespace']);
  $form_state['default_relationship'] = $policy->getRelationship();
  return array(
    '#action' => request_uri() . '#create-child-collection',
    'label' => array(
      '#type' => 'textfield',
      '#title' => t('Collection Label'),
      '#description' => t('Human readable name for this collection'),
      '#required' => TRUE,
      '#size' => 25,
    ),
    'pid' => array(
      '#type' => 'textfield',
      '#title' => t('Collection PID'),
      '#description' => t("Unique PID for this collection. Leave blank for default.<br/>PID's take the general form of namespace:collection (eg. islandora:pamphlets)"),
      '#size' => 15,
    ),
    'inherit_policy' => array(
      '#type' => 'checkbox',
      '#title' => t('Inherit collection policy?'),
      '#default_value' => TRUE,
    ),
    'policy' => array(
      '#type' => 'fieldset',
      '#title' => t('Collection Policy'),
      '#states' => array(
        'visible' => array(
          ':input[name="inherit_policy"]' => array('checked' => FALSE),
        ),
      ),
      'namespace' => islandora_basic_collection_get_namespace_form_element($form_state['default_namespace']),
      'content_models' => array(
        '#title' => "Choose allowable content models for this collection",
        '#type' => 'checkboxes',
        '#options' => islandora_basic_collection_get_content_models_as_form_options($content_models),
        '#description' => t("Content models describe the behaviours of objects with which they are associated."),
      ),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Create collection'),
    ),
  );
}

/**
 * Validate handler for create child collection form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_create_child_collection_form_validate(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  if (!empty($form_state['values']['pid'])) {
    $pid = $form_state['values']['pid'];
    if (!islandora_is_valid_pid($pid)) {
      form_set_error('pid', t('Collection PID is Invalid.'));
    }
    elseif (islandora_object_load($pid)) {
      form_set_error('pid', t('Collection PID already exists.'));
    }
  }
}

/**
 * Submit handler for create child collection form.
 *
 * Creates a new collection object with COLLECTION_POLICY and TN datastreams.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_create_child_collection_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $collection = $form_state['collection'];
  $pid = empty($form_state['values']['pid']) ? $form_state['default_namespace'] : $form_state['values']['pid'];
  // Prepare Object.
  $child_collection = islandora_prepare_new_object($pid, $form_state['values']['label'],
                      array(),
                      array('islandora:collectionCModel'),
                      array(array(
                          'pid' => $collection->id,
                          'relationship' => $form_state['default_relationship'])));
  // Add TN datastream.
  $thumbnail_url = url(drupal_get_path('module', 'islandora_basic_collection') . '/images/folder.png', array('absolute' => TRUE));
  $thumbnail_datastream = $child_collection->constructDatastream('TN');
  $thumbnail_datastream->setContentFromUrl($thumbnail_url);
  $thumbnail_datastream->label = 'Thumbnail';
  $thumbnail_datastream->mimetype = 'image/png';
  $child_collection->ingestDatastream($thumbnail_datastream);
  // Add COLLECTION_POLICY datastream.
  if ($form_state['values']['inherit_policy']) {
    $collection_policy = $collection['COLLECTION_POLICY']->content;
  }
  else {
    $policy = CollectionPolicy::emptyPolicy();
    $content_models = array_filter($form_state['values']['content_models']);
    foreach (array_keys($content_models) as $pid) {
      $policy->addContentModel($pid, $form_state['content_models'][$pid]['label'], $form_state['values']['namespace']);
    }
    $collection_policy = $policy->getXML();
  }
  $policy_datastream = $child_collection->constructDatastream('COLLECTION_POLICY', 'X');
  $policy_datastream->setContentFromString($collection_policy);
  $policy_datastream->label = 'Collection policy';
  $child_collection->ingestDatastream($policy_datastream);
  // Ingest the new object.
  islandora_add_object($child_collection);
  drupal_goto('/islandora/object/' . $child_collection->id);
}

/**
 * Define collection policy management form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The collection to manipulate.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_policy_management_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $form_state['collection'] = $object;
  $policy = new CollectionPolicy($object['COLLECTION_POLICY']->content);
  $policy_content_models = $policy->getContentModels();
  $content_models = islandora_get_content_models();
  $default_namespace = drupal_substr($object->id, 0, strpos($object->id, ":"));
  $rows = array();
  foreach ($content_models as $pid => $content_model) {
    $label = $content_model['label'];
    $content_model_in_policy = isset($policy_content_models[$pid]);
    $namespace = $content_model_in_policy ? $policy_content_models[$pid]['namespace'] : $default_namespace;
    $namespace_element = islandora_basic_collection_get_namespace_form_element($namespace);
    unset($namespace_element['#title'],$namespace_element['#description']);
    $rows[$pid] = array(
      'selected' => array(
        '#type' => 'checkbox',
        '#default_value' => $content_model_in_policy,
      ),
      'title' => array(
        '#markup' => l(t('@label (@pid)', array('@label' => $label, '@pid' => $pid)), "islandora/object/{$pid}"),
      ),
      'namespace' => $namespace_element,
    );
  }
  return array(
    '#action' => request_uri() . '#policy-management',
    'help' => array(
      '#markup' => l(t('About Collection Policies'), 'https://wiki.duraspace.org/display/ISLANDORA112/How+to+Manage+Collection+Policies', array()),
    ),
    'table' => array(
      '#tree' => TRUE,
      '#header' => array(
        theme('table_select_header_cell'), t('PID'), 'Namespace',
      ),
      '#theme' => 'islandora_basic_collection_policy_management_table',
      'rows' => $rows,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update Collection Policy'),
    ),
  );
}

/**
 * Submit handler for collection policy management form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_policy_management_form_submit(array $form, array &$form_state) {
  $collection = $form_state['collection'];
  $filter_selected = function($o) {
    return $o['selected'];
  };
  $selected = array_filter($form_state['values']['table']['rows'], $filter_selected);
  $policy = CollectionPolicy::emptyPolicy();
  foreach ($selected as $pid => $properties) {
    $content_model = islandora_object_load($pid);
    $policy->addContentModel($pid, $content_model->label, $properties['namespace']);
  }
  $collection['COLLECTION_POLICY']->content = $policy->getXML();
  drupal_set_message(t('Updated Collection Policy'));
}

/**
 * Change the supported content models for the given collection.
 *
 * Modifies the collections COLLECTION_POLICY datastream.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The collection to add the child collection to.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_change_content_models_form(array $form, array &$form_state, FedoraObject $object) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $form_state['collection'] = $object;
  $policy = new CollectionPolicy($object['COLLECTION_POLICY']->content);
  $form_state['policy_content_models'] = $policy_content_models = $policy->getContentModels();
  $form_state['content_models'] = $content_models = islandora_get_content_models();
  $show_namespace_for_content_models = array_keys(array_diff_key($content_models, $policy_content_models));
  $default_namespace = drupal_substr($object->id, 0, strpos($object->id, ":"));
  $namespace = islandora_basic_collection_get_namespace_form_element($default_namespace);
  $namespace['#element_validate'] = array('islandora_basic_collection_change_content_models_form_validate_namespace');
  $namespace['#states'] = array('visible' => array());
  foreach ($show_namespace_for_content_models as $content_model) {
    $namespace['#states']['visible'][] = array(
      ":input[name='new_content_model'],value='$content_model'" => array('value' => $content_model),
    );
  }
  return array(
    '#action' => request_uri() . '#change-content-models',
    'current_content_model' => array(
      '#title' => "Choose content model to be changed",
      '#type' => 'select',
      '#options' => islandora_basic_collection_get_content_models_as_form_options($policy_content_models),
      '#description' => t("All objects in this collection with the selected content model will be changed."),
    ),
    'new_content_model' => array(
      '#title' => "Choose new content model",
      '#type' => 'select',
      '#options' => islandora_basic_collection_get_content_models_as_form_options($content_models),
      '#description' => t("The new content model to be assigned to selected objects."),
    ),
    'namespace' => $namespace,
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Change content model associations'),
    ),
  );
}

/**
 * Validates the namespace form element.
 *
 * @param array $element
 *   The namespace form element.
 * @param array $form_state
 *   The Drupal form state.
 * @param array $form
 *   The Drupal form definition.
 */
function islandora_basic_collection_change_content_models_form_validate_namespace(array $element, array &$form_state, array $form) {
  // Only use the namespace for content models not in the policy.
  if (in_array($form_state['values']['new_content_model'], array_keys($form_state['policy_content_models']))) {
    return;
  }
  if (empty($element['#value'])) {
    form_error($element, t('This field is required.'));
  }
  elseif (!islandora_is_valid_namespace($element['#value'])) {
    form_error($element, t('The namespace is not valid.'));
  }
}

/**
 * Validate handler for the change content models form.
 *
 * Checks the DS-COMPOSITE-MODEL of both the existing and new content model
 * such that all required datastreams in the new content model are also
 * required in the existing content model.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_change_content_models_form_validate(array $form, array &$form_state) {
  $current = islandora_object_load($form_state['values']['current_content_model']);
  $new = islandora_object_load($form_state['values']['new_content_model']);
  // Both content models must define a DS-COMPOSITE-MODEL.
  if (empty($current['DS-COMPOSITE-MODEL'])) {
    $message = t('Content model @pid does not have a DS-COMPOSITE-MODEL datastream.', array('@pid' => $current->id));
    form_set_error('current_content_model', $message);
  }
  elseif (empty($new['DS-COMPOSITE-MODEL'])) {
    $message = t('Content model @pid does not have a DS-COMPOSITE-MODEL datastream.', array('@pid' => $new->id));
    form_set_error('new_content_model', $message);
  }
  $filter_required = function($o) {
    return !$o['optional'];
  };
  $current_requirements = islandora_get_datastreams_requirements_from_content_model($current);
  $current_requirements = array_filter($current_requirements, $filter_required);
  $new_requirements = islandora_get_datastreams_requirements_from_content_model($new);
  $new_requirements = array_filter($new_requirements, $filter_required);
  // The require datastreams in the new content model must also be required in
  // the current content model.
  if ($new_requirements != array_intersect_key($new_requirements, $current_requirements)) {
    $message_parameters = array('@new' => $new->id, '@current' => $current->id);
    $message = t('Content model @new has required datastreams that @current does not have.', $message_parameters);
    form_set_error('new_content_model', $message);
  }
  else {
    // The requirements must also be identical.
    foreach ($new_requirements as $dsid => $requirement) {
      if ($requirement != $current_requirements[$dsid]) {
        $message_parameters = array('@new' => $new->id, '@current' => $current->id);
        $message = t('Content model @new required datastreams do not match @current.', $message_parameters);
        form_set_error('new_content_model', $message);
        break;
      }
    }
  }
}

/**
 * Submit handler for the change content models form.
 *
 * Updates the collection's COLLECTION_POLICY datastream with the changes
 * made in the form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_change_content_models_form_submit(array $form, array &$form_state) {
  $collection = $form_state['collection'];
  $current = $form_state['values']['current_content_model'];
  $new = $form_state['values']['new_content_model'];
  if (!in_array($new, array_keys($form_state['policy_content_models']))) {
    // Add to Policy if not already present.
    $policy = new CollectionPolicy($collection['COLLECTION_POLICY']->content);
    $policy->addContentModel($new, $form_state['content_models'][$new]['label'], $form_state['values']['namespace']);
    $collection['COLLECTION_POLICY']->content = $policy->getXML();
  }
  $current = islandora_object_load($current);
  $new = islandora_object_load($new);
  $batch = islandora_basic_collection_change_content_model_batch($collection, $current, $new);
  batch_set($batch);
}


/**
 * Define the migrate children form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The collection to move child objects from.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_migrate_children_form(array $form, array &$form_state, FedoraObject $object) {
  $form_state['collection'] = $object;
  $map_new_collection_options = function($o) {
    return $o['label'];
  };
  $new_collection_options = array_map($map_new_collection_options, islandora_basic_collection_get_collections());
  // Can't copy/migrate to my self.
  unset($new_collection_options[$object->id]);
  return array(
    '#action' => request_uri() . '#migrate-children',
    'action' => array(
      '#title' => t('Move or copy the selected objects'),
      '#type' => 'radios',
      '#options' => array(
        'move' => 'Move',
        'copy' => 'Copy',
      ),
      '#default_value' => 'copy',
      '#description' => t('Objects can belong to one or many collections.'),
    ),
    'new_collection' => array(
      '#title' => t('To new collection'),
      '#description' => t("All content will be migrated from @pid to the selected collection", array('@pid' => $object->id)),
      '#type' => 'select',
      '#options' => $new_collection_options,
    ),
    'children' => islandora_basic_collection_get_children_select_table_form_element($object, array(
                  'element' => 0,
                  'fragment' => '#migrate-children',
                )),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Migrate selected objects'),
    ),
  );
}

/**
 * Submit handler for the migrate children form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_migrate_children_form_submit(array $form, array &$form_state) {
  $collection = $form_state['collection'];
  $new_collection = islandora_object_load($form_state['values']['new_collection']);
  $children = array_keys(array_filter($form_state['values']['children']));
  if ($form_state['values']['action'] == 'move') {
    $batch = islandora_basic_collection_migrate_children_batch($collection, $new_collection, $children);
  }
  else {
    $batch = islandora_basic_collection_copy_children_batch($collection, $new_collection, $children);
  }
  batch_set($batch);
}

/**
 * Delete child objects form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param FedoraObject $object
 *   The collection to add the child collection to.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_delete_children_form(array $form, array &$form_state, FedoraObject $object) {
  $form_state['collection'] = $object;
  return array(
    '#action' => request_uri() . '#delete-children',
    'children' => islandora_basic_collection_get_children_select_table_form_element($object, array(
                  'element' => 1,
                  'fragment' => '#delete-children',
                )),
    'description' => array(
      '#type' => 'item',
      '#markup' => t('Are you sure you want to delete the selected objects?<br/>This action cannot be undone.'),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Delete selected objects'),
    ),
  );
}

/**
 * Submit handler for  object deletion form in the collection manager.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_delete_children_form_submit(array $form, array &$form_state) {
  $collection = $form_state['collection'];
  $children = array_keys(array_filter($form_state['values']['children']));
  $batch = islandora_basic_collection_delete_children_batch($collection, $children);
  batch_set($batch);
}
