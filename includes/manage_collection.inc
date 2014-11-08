<?php

/**
 * @file
 * Handles the management of collection objects and their children.
 */

require_once dirname(__FILE__) . '/utilities.inc';
require_once dirname(__FILE__) . '/batch.inc';

// Hook definition.
define('ISLANDORA_BASIC_COLLECTION_BUILD_MANAGE_OBJECT_HOOK', 'islandora_basic_collection_build_manage_object');

/**
 * Manage Collection local task.
 *
 * Defines the actions to appear in the collection section of the Manage tab.
 *
 * @param AbstractObject $object
 *   The collection object.
 *
 * @return array
 *   A renderable array.
 */
function islandora_basic_collection_manage_object(AbstractObject $object) {
  $return_form = array(
    'manage_collection_object' => array(),
  );
  $data = islandora_invoke_hook_list(ISLANDORA_BASIC_COLLECTION_BUILD_MANAGE_OBJECT_HOOK,
    $object->models, array(
      $return_form,
      $object)
  );
  $data['manage_collection_object']['#type'] = 'vertical_tabs';
  return $data;
}

/**
 * Implements islandora_basic_collection_build_manage_object_hook().
 *
 * @param array $form_state
 *   The Form state of the form.
 * @param AbstractObject $object
 *   The Islandora object being processed.
 *
 * @return array
 *   Returning the built form.
 */
function islandora_basic_collection_islandora_basic_collection_build_manage_object(array $form_state, AbstractObject $object) {
  // Defines accessible elements.
  $access = array(
    'policy_management' => islandora_object_access(ISLANDORA_BASIC_COLLECTION_MANAGE_COLLECTION_POLICY, $object),
    'share_children' => islandora_object_access(ISLANDORA_BASIC_COLLECTION_MIGRATE_COLLECTION_MEMBERS, $object),
    'migrate_children' => islandora_object_access(ISLANDORA_BASIC_COLLECTION_MIGRATE_COLLECTION_MEMBERS, $object),
    'delete_children' => islandora_object_access(ISLANDORA_PURGE, $object),
  );
  // Build forms for only the accessible elements.
  foreach ($access as $element => $accessible) {
    $form = "islandora_basic_collection_{$element}_form";
    $forms[$element] = $accessible ? drupal_get_form($form, $object) : NULL;
  }
  $form_state['manage_collection_object']['policy_management'] = array(
    '#id' => 'policy-management',
    '#group' => 'manage_collection_object',
    '#access' => $access['policy_management'],
    '#type' => 'fieldset',
    '#title' => t('Manage collection policy'),
    'form' => $forms['policy_management'],
  );
  $form_state['manage_collection_object']['share_children'] = array(
    '#id' => 'share-children',
    '#group' => 'manage_collection_object',
    '#access' => $access['share_children'],
    '#type' => 'fieldset',
    '#title' => t('Share members'),
    'form' => $forms['share_children'],
  );
  $form_state['manage_collection_object']['migrate_children'] = array(
    '#id' => 'migrate-children',
    '#group' => 'manage_collection_object',
    '#access' => $access['migrate_children'],
    '#type' => 'fieldset',
    '#title' => t('Migrate members'),
    'form' => $forms['migrate_children'],
  );
  $form_state['manage_collection_object']['delete_children'] = array(
    '#id' => 'delete-children',
    '#group' => 'manage_collection_object',
    '#access' => $access['delete_children'],
    '#type' => 'fieldset',
    '#title' => t('Delete members of this collection'),
    'form' => $forms['delete_children'],
  );
  return $form_state;
}

/**
 * Define child collection form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_create_child_collection_form(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $parent_object = islandora_object_load($form_state['islandora']['shared_storage']['parent']);
  // Permissions handling.
  if (!user_access(ISLANDORA_BASIC_COLLECTION_CREATE_CHILD_COLLECTION)) {
    drupal_set_message(t('You do not have permissions to create collections.'), 'error');
    drupal_goto('islandora/object/' . $parent_object->id);
  }
  $policy = new CollectionPolicy($parent_object['COLLECTION_POLICY']->content);
  $policy_content_models = $policy->getContentModels();
  $content_models = islandora_get_content_models();
  $form_state['content_models'] = $content_models;
  $default_namespace = islandora_get_namespace($policy_content_models['islandora:collectionCModel']['namespace']);
  return array(
    '#action' => request_uri() . '#create-child-collection',
    'label' => array(
      '#type' => 'textfield',
      '#title' => t('Collection Label'),
      '#description' => t('Human-readable name for this collection.'),
      '#required' => TRUE,
      '#size' => 25,
    ),
    'pid' => array(
      '#type' => 'textfield',
      '#title' => t('Collection PID'),
      '#description' => t("Unique PID for this collection. Leave blank to use the default.<br/>PIDs take the general form of <strong>namespace:collection</strong> (e.g., islandora:pamphlets)"),
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
      'namespace' => islandora_basic_collection_get_namespace_form_element($default_namespace),
      'content_models' => array(
        '#title' => "Allowable content models",
        '#type' => 'checkboxes',
        '#options' => islandora_basic_collection_get_content_models_as_form_options($content_models),
        '#description' => t("Content models describe the behaviours of objects with which they are associated."),
      ),
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
      form_set_error('pid', t('Collection PID is invalid.'));
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
  // Prepare Object.
  $new_collection = $form_state['islandora']['objects'][0];
  if (!empty($form_state['values']['pid'])) {
    $new_collection->id = $form_state['values']['pid'];
  }
  $new_collection->label = $form_state['values']['label'];

  // Add COLLECTION_POLICY datastream.
  $parent_collection = islandora_object_load($form_state['islandora']['shared_storage']['parent']);
  if ($form_state['values']['inherit_policy']) {
    $collection_policy = $parent_collection['COLLECTION_POLICY']->content;
  }
  else {
    $policy = CollectionPolicy::emptyPolicy();
    $content_models = array_filter($form_state['values']['content_models']);
    foreach (array_keys($content_models) as $pid) {
      $policy->addContentModel($pid, $form_state['content_models'][$pid]['label'], $form_state['values']['namespace']);
    }
    $collection_policy = $policy->getXML();
  }
  $policy_datastream = $new_collection->constructDatastream('COLLECTION_POLICY', 'X');
  $policy_datastream->setContentFromString($collection_policy);
  $policy_datastream->label = 'Collection policy';
  $new_collection->ingestDatastream($policy_datastream);

}

/**
 * Define collection policy management form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The collection to manipulate.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_policy_management_form(array $form, array &$form_state, AbstractObject $object) {
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
    unset($namespace_element['#title'], $namespace_element['#description']);
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
      '#type' => 'item',
      '#markup' => l(t('About Collection Policies'), 'https://wiki.duraspace.org/display/ISLANDORA713/How+to+Manage+Collection+Policies', array()),
    ),
    'table' => array(
      '#tree' => TRUE,
      '#header' => array(
        array('class' => array('select-all')), t('PID'), 'Namespace',
      ),
      '#theme' => 'islandora_basic_collection_policy_management_table',
      'rows' => $rows,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Update collection policy'),
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
  drupal_set_message(t('Updated collection policy'));
}

/**
 * Define the share children form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The collection to share child objects from.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_share_children_form(array $form, array &$form_state, AbstractObject $object) {
  $form_state['collection'] = $object;
  $fragment = '#share-children';
  return array(
    '#action' => request_uri() . $fragment,
    'collection' => array(
      '#title' => t('Share members with collection'),
      '#description' => t("Members can be shared with any number of collections."),
      '#type' => 'select',
      '#options' => islandora_basic_collection_get_other_collections_as_form_options($object),
    ),
    'children' => islandora_basic_collection_get_children_select_table_form_element($object, array(
                  'element' => 0,
                  'fragment' => $fragment,
                )),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Share selected objects'),
    ),
    'submit_all' => array(
      '#type' => 'submit',
      '#value' => t('Share All Objects'),
    ),
  );
}

/**
 * Submit handler for the share children form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_share_children_form_submit(array $form, array &$form_state) {
  $collection = $form_state['collection'];
  $new_collection = islandora_object_load($form_state['values']['collection']);
  $clicked_button = end($form_state['clicked_button']['#parents']);
  if ($clicked_button == 'submit_all') {
    $batch = islandora_basic_collection_share_children_batch($collection, $new_collection, NULL);
  }
  else {
    $children = array_keys(array_filter($form_state['values']['children']));
    $batch = islandora_basic_collection_share_children_batch($collection, $new_collection, $children);
  }

  batch_set($batch);
}

/**
 * Define the migrate children form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $object
 *   The collection to move child objects from.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_migrate_children_form(array $form, array &$form_state, AbstractObject $object) {
  $form_state['collection'] = $object;
  $fragment = '#migrate-children';
  return array(
    '#action' => request_uri() . $fragment,
    'collection' => array(
      '#title' => t('Migrate members to collection'),
      '#description' => t('Removes members from their current collection (%label) and adds them to the selected collection.', array('%label' => $object->label)),
      '#type' => 'select',
      '#options' => islandora_basic_collection_get_other_collections_as_form_options($object),
    ),
    'children' => islandora_basic_collection_get_children_select_table_form_element($object, array(
                  'element' => 1,
                  'fragment' => $fragment,
                )),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Migrate selected objects'),
    ),
    'submit_all' => array(
      '#type' => 'submit',
      '#value' => t('Migrate All Objects'),
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
  $new_collection = islandora_object_load($form_state['values']['collection']);
  $clicked_button = end($form_state['clicked_button']['#parents']);
  if ($clicked_button == 'submit_all') {
    $batch = islandora_basic_collection_migrate_children_batch($collection, $new_collection, NULL);
  }
  else {
    $children = array_keys(array_filter($form_state['values']['children']));
    $batch = islandora_basic_collection_migrate_children_batch($collection, $new_collection, $children);
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
 * @param AbstractObject $object
 *   The collection to add the child collection to.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_delete_children_form(array $form, array &$form_state, AbstractObject $object) {
  $form_state['collection'] = $object;
  return array(
    '#action' => request_uri() . '#delete-children',
    'children' => islandora_basic_collection_get_children_select_table_form_element($object, array(
                  'element' => 2,
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
