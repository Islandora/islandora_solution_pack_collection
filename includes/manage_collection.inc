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
    'regenerate_derivatives' => islandora_object_access(ISLANDORA_REGENERATE_DERIVATIVES, $object),
    'regenerate_metadata' => islandora_object_access(ISLANDORA_REGENERATE_DERIVATIVES, $object),
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
  $form_state['manage_collection_object']['regenerate_derivatives'] = array(
    '#id' => 'regenerate-derivatives',
    '#group' => 'manage_collection_object',
    '#access' => $access['regenerate_derivatives'],
    '#type' => 'fieldset',
    '#title' => t('Regenerate child derivatives'),
    'form' => $forms['regenerate_derivatives'],
  );
  if (module_exists('xml_form_builder')) {
    $form_state['manage_collection_object']['regenerate_metadata'] = array(
      '#id' => 'regenerate-metadata',
      '#group' => 'manage_collection_object',
      '#access' => $access['regenerate_metadata'],
      '#type' => 'fieldset',
      '#title' => t('Regenerate child DC metadata'),
      'form' => $forms['regenerate_metadata'],
    );
  }
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

/**
 * Derivative regeneration form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $collection
 *   The collection to generate the form for.
 *
 * @return array
 *   A form containing controls to trigger derivative regeneration.
 */
function islandora_basic_collection_regenerate_derivatives_form(array $form, array &$form_state, AbstractObject $collection) {
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  $form_state['collection'] = $collection;
  $content_models = islandora_basic_collection_get_content_models_as_form_options(islandora_basic_collection_get_child_content_models($collection));
  $content_model = islandora_basic_collection_get_current_content_model($collection, $form_state);
  $form = array(
    'regenerate_derivatives_wrapper' => array(
      '#action' => request_uri() . "#regenerate-derivatives",
      '#prefix' => '<div id="regenerate-derivatives-wrapper">',
      '#suffix' => '</div>',
      'content_model' => array(
        '#type' => 'select',
        '#title' => t('Content model to target'),
        '#options' => $content_models,
        '#default_value' => $content_model,
        '#ajax' => array(
          'callback' => 'islandora_basic_collection_update_regenerate_derivatives_list',
          'wrapper' => 'regenerate-derivatives-wrapper',
          'effect' => 'fade',
          'event' => 'change',
          'progress' => array('type' => 'throbber'),
        ),
      ),
      'dsids' => array(
        '#type' => 'tableselect',
        '#header' => array(
          'dsid' => t('DSID'),
        ),
        '#options' => islandora_basic_collection_regenerate_derivatives_form_get_derivatives_list($content_model),
        '#empty' => t('No derivative DSIDs for the selected content model.'),
      ),
      'children' => islandora_basic_collection_get_children_select_table_form_element($collection, array(
                    'element' => 3,
                    'fragment' => '#regenerate-derivatives',
                  ), $content_model),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Regenerate selected derivatives for selected objects'),
      ),
      'submit_all' => array(
        '#type' => 'submit',
        '#value' => t('Regenerate selected derivatives for all collection objects'),
      ),
    ),
  );
  $form['regenerate_derivatives_wrapper']['children']['#empty'] = t("No children found for the selected content model.");
  return $form;
}

/**
 * Callback to update the regenerate derivatives form when the cmodel changes.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The 'regenerate_derivatives_wrapper' section of the form.
 */
function islandora_basic_collection_update_regenerate_derivatives_list(array $form, array &$form_state) {
  return $form["regenerate_derivatives_wrapper"];
}

/**
 * Gets a list of derivatives specified by the given content model.
 *
 * These are returned in a format appropriate for the regenerate derivatives
 * 'dsids' tableselect field.
 *
 * @param string $content_model
 *   The content model to retrieve derivatives for.
 *
 * @return array
 *   An array of DSIDs that can be used in the regenerate derivatives 'dsids'
 *   tableselect field.
 */
function islandora_basic_collection_regenerate_derivatives_form_get_derivatives_list($content_model) {
  $content_model = islandora_object_load($content_model);
  if ($content_model) {
    module_load_include('inc', 'islandora', 'includes/utilities');
    $dsids = array();
    foreach (islandora_invoke_hook_list(ISLANDORA_DERVIATIVE_CREATION_HOOK, array($content_model), array()) as $derivative_hook) {
      $dsids[$derivative_hook['destination_dsid']] = array('dsid' => $derivative_hook['destination_dsid']);
    }
    return $dsids;
  }
  return array();
}

/**
 * Gets the currently selected content model in the form state.
 *
 * @param AbstractObject $collection
 *   The collection we are getting the current content model for.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return string
 *   The content model currently selected in the 'content_model' select field
 *   of the regenerate derivatives form.
 */
function islandora_basic_collection_get_current_content_model(AbstractObject $collection, array $form_state) {
  if (isset($form_state['values'])) {
    if (isset($form_state['values']['content_model'])) {
      return $form_state['values']['content_model'];
    }
  }
  $params = drupal_get_query_parameters();
  if (isset($params['content_model'])) {
    return $params['content_model'];
  }
  $content_models = islandora_basic_collection_get_content_models_as_form_options(islandora_basic_collection_get_child_content_models($collection));
  reset($content_models);
  return key($content_models);
}

/**
 * Validation for the regenerate derivatives form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_regenerate_derivatives_form_validate(array $form, array &$form_state) {
  $clicked_button = end($form_state['clicked_button']['#parents']);
  // At least one DSID must be selected to continue.
  $dsids = array_keys(array_filter($form_state['values']['dsids']));
  if (empty($dsids)) {
    form_set_error('dsids', t('You must select at least one DSID.'));
  }
  // At least one child must be selected to continue if the regular 'submit'
  // button was clicked instead of 'submit_all'.
  $children = array_keys(array_filter($form_state['values']['children']));
  if (empty($children) && $clicked_button === 'submit') {
    form_set_error('children', t('You must either select at least one child, or regenerate derivatives for all objects.'));
  }
}

/**
 * Submit handler for the regenerate derivatives form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_regenerate_derivatives_form_submit(array $form, array &$form_state) {
  $clicked_button = end($form_state['clicked_button']['#parents']);
  $collection = $form_state['collection'];
  if ($clicked_button === 'submit_all') {
    $children = NULL;
  }
  else {
    $children = array_keys(array_filter($form_state['values']['children']));
  }
  module_load_include('inc', 'islandora_basic_collection', 'includes/batch');
  $batch = islandora_basic_collection_regenerate_derivatives_batch($children, array_keys(array_filter($form_state['values']['dsids'])), $collection, $form_state['values']['content_model']);
  batch_set($batch);
}

/**
 * The form for regenerating metadata.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param AbstractObject $collection
 *   The collection to manipulate child metadata for.
 *
 * @return array
 *   A form containing controls for regenerating child metadata.
 */
function islandora_basic_collection_regenerate_metadata_form(array $form, array &$form_state, AbstractObject $collection) {
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  module_load_include('inc', 'xml_form_builder', 'includes/db');
  $form_state['collection'] = $collection;
  $content_models = islandora_basic_collection_get_content_models_as_form_options(islandora_basic_collection_get_child_content_models($collection));
  $content_model = islandora_basic_collection_get_current_content_model($collection, $form_state);
  $form = array(
    '#action' => request_uri() . '#regenerate-metadata',
    'regenerate_metadata_wrapper' => array(
      '#prefix' => '<div id="regenerate-metadata-wrapper">',
      '#suffix' => '</div>',
      'content_model' => array(
        '#type' => 'select',
        '#title' => t('Content model to target'),
        '#options' => $content_models,
        '#default_value' => $content_model,
        '#ajax' => array(
          'callback' => 'islandora_basic_collection_update_regenerate_metadata_list',
          'wrapper' => 'regenerate-metadata-wrapper',
          'effect' => 'fade',
          'event' => 'change',
          'progress' => array('type' => 'throbber'),
        ),
      ),
      'mapping' => array(
        '#type' => 'select',
        '#title' => t('Mapping to use'),
        '#options' => islandora_basic_collection_regenerate_metadata_get_dc_xslts($content_model),
      ),
      'children' => islandora_basic_collection_get_children_select_table_form_element($collection, array(
        'element' => 4,
        'fragment' => '#regenerate-metadata',
      ), $content_model),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Regenerate DC metadata for selected objects'),
      ),
      'submit_all' => array(
        '#type' => 'submit',
        '#value' => t('Regenerate DC metadata for all collection objects'),
      ),
    ),
  );
  $form['regenerate_metadata_wrapper']['children']['#empty'] = t("No children found for the selected content model.");
  return $form;
}

/**
 * Callback to update the regenerate metadata form when the cmodel changes.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The 'regenerate_metadata_wrapper' section of the form.
 */
function islandora_basic_collection_update_regenerate_metadata_list(array $form, array $form_state) {
  return $form["regenerate_metadata_wrapper"];
}

/**
 * Gets the list of either default DC XSLTs or hooked DC XSLTs.
 *
 * @param string $content_model
 *   The content model to get default DC XSLTs for.
 *
 * @return array
 *   An array of XSLTs for the current content model.
 */
function islandora_basic_collection_regenerate_metadata_get_dc_xslts($content_model) {
  $xslts = array();
  module_load_include('inc', 'xml_form_builder', 'includes/db');
  if (variable_get('xml_form_builder_use_default_dc_xslts', FALSE)) {
    $mappings = xml_form_builder_get_default_dc_xslt_mappings();
    foreach ($mappings as $mapping) {
      if ($mapping['content_model'] === $content_model) {
        $xslts[$mapping['id']] = $mapping['name'];
      }
    }
  }
  else {
    $associations = module_invoke_all('islandora_xml_form_builder_form_associations');
    // @TODO: This is supposed to be deprecated eventually.
    $associations += module_invoke_all('islandora_content_model_forms_form_associations');
    foreach ($associations as $association) {
      if ($association['content_model'] === $content_model) {
        $xslts["{$association['transform']}:{$association['dsid']}"] = "{$association['transform']} - {$association['dsid']}";
      }
    }
    $xslts = array_unique($xslts);
  }
  return $xslts;
}

/**
 * Validation for the regenerate metadata form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_regenerate_metadata_form_validate(array $form, array &$form_state) {
  $clicked_button = end($form_state['clicked_button']['#parents']);
  // At least one child must be selected to continue if the regular 'submit'
  // button was clicked instead of 'submit_all'.
  $children = array_keys(array_filter($form_state['values']['children']));
  if (empty($children) && $clicked_button === 'submit') {
    form_set_error('children', t('You must either select at least one child, or regenerate metadata for all objects.'));
  }
  if (!$form_state['values']['mapping']) {
    form_set_error('mapping', t('No metadata mapping was selected.'));
  }
}

/**
 * Submit handler for the regenerate metadata form.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_regenerate_metadata_form_submit(array $form, array &$form_state) {
  $clicked_button = end($form_state['clicked_button']['#parents']);
  $collection = $form_state['collection'];
  if ($clicked_button === 'submit_all') {
    $children = NULL;
  }
  else {
    $children = array_keys(array_filter($form_state['values']['children']));
  }
  module_load_include('inc', 'islandora_basic_collection', 'includes/batch');
  $batch = islandora_basic_collection_regenerate_metadata_batch($children, $form_state['values']['mapping'], $collection, $form_state['values']['content_model']);
  batch_set($batch);
}
