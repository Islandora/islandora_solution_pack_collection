<?php

/**
 * @file
 * ManagePolicies.inc
 */

/**
 * Create form to collection policy management feature
 *
 * @param array $form
 * @param array $form_state
 * @param string $collection_pid
 * @return type
 */
function islandora_manage_policies_form($form, &$form_state, $collection_object) {
  module_load_include('inc', 'islandora_basic_collection', 'includes/CollectionPolicy');

  $repository = $collection_object->repository;

  $content_models = get_content_models_list($collection_object);
  $cm_options = array();
  $name_mappings = array();
  foreach ($content_models as $content_model) {
    if ($content_model != "islandora:collectionCModel") {
      try {
        $item = $repository->getObject($content_model);
        $cm_name = $item->Label;
        $cm_options[$content_model] = $cm_name;
      } catch (RepositoryException $e) {

      }
    }
  }

  $namespace = drupal_substr($collection_object->id, 0, strpos($collection_object->id, ":"));

  $collection_policy_dsid = variable_get('Islandora_Collection_Policy_DSID', 'COLLECTION_POLICY');
  $collection_policy_string = $collection_object->getDatastream($collection_policy_dsid);

  $supported_collection_models = array();
  if ($collection_policy_string) {
    $collection_policy = new CollectionPolicy($collection_policy_string->content);
    $supported_collection_models = $collection_policy->getContentModels();
  }

  $collection_namespace = drupal_substr($collection_object->id, 0, strpos($collection_object->id, ":"));

  $represented_content_models = get_represented_content_models($collection_object);
  $collection_name = $collection_object->label;
  $new_content_models = get_content_models_as_option_array();
  $current_models_in_policy = array();

  if ($supported_collection_models) {
    foreach ($supported_collection_models as $supported_model) {
      $current_models_in_policy[$supported_model['pid']] = $supported_model['pid'];
    }
  }

  $new_options = array();
  if (is_array($new_content_models) && is_array($current_models_in_policy)) {
    $new_options = array_diff_key($new_content_models, $current_models_in_policy);
  }

  $s = '';
  if (count($current_models_in_policy) > 1) {
    $s = 's';
  }

  $form['manage_collection_policy']['titlebox'] = array(
    '#type' => 'item',
    '#title' => t("Manage collection policy for @collection_title", array('@collection_title' => $collection_object->label)),
  );

  if (count($new_content_models) > 0) {
    $form ['manage_collection_policy']['add']['content_model_to_add'] = array(
      '#title' => "Choose content model",
      '#type' => 'select',
      '#options' => $new_options,
      '#description' => t("Choose content model to add to this collection policy."),
    );

    $form['manage_collection_policy']['add']['new_cp_namespace'] = array(
      '#title' => "Choose namespace",
      '#type' => 'textfield',
      '#size' => 15,
      '#default_value' => $namespace,
      '#description' => t("Choose namespace for objects in this collection associated with this content model"),
    );

    $form['parent_collection'] = array(
      '#type' => 'hidden',
      '#value' => $collection_object->id,
    );

    $form['collection_pid'] = array(
      '#type' => 'hidden',
      '#value' => $collection_object->id,
    );

    $form['manage_collection_policy']['add']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Add content model to collection policy'),
      '#id' => 'add_cm'
    );
  }
  else {
    $form['manage_collection_policy']['no_content_models'] = array(
      '#markup' => t('No content models available to add!'),
    );
  }

  if (count($current_models_in_policy) > 0) {
    $form['manage_collection_policy']['remove'] = array(
      '#title' => "Delete content model from collection policy",
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
    );

    $form ['manage_collection_policy']['remove']['content_models_to_remove'] = array(
      '#title' => "Choose content model to remove",
      '#type' => 'checkboxes',
      '#options' => $current_models_in_policy,
      '#description' => t("Choose content models to remove from this collection policy."),
    );

    $form['manage_collection_policy']['remove']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Remove content model@s from collection policy', array('@s' => $s)),
      '#id' => 'remove_cm'
    );
  }
  return $form;
}

function islandora_manage_policies_form_validate($form, &$form_state) {

}

/**
 * Submit handler for collection policy management form
 *
 * @param array $form
 * @param array $form_state
 */
function islandora_manage_policies_form_submit($form, &$form_state) {
  module_load_include('module', 'islandora', 'islandora');
  $collection_pid = $form_state['values']['parent_collection'];
  $collection_object = islandora_object_load($collection_pid);

  $policy = $collection_object->getDatastream(variable_get('Islandora_Collection_Policy_DSID', 'COLLECTION_POLICY'));
  $collection_policy = '<?xml version="1.0" encoding="UTF-8"?>
<collection_policy xmlns="http://www.islandora.ca" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" name="" xsi:schemaLocation="http://www.islandora.ca http://syn.lib.umanitoba.ca/collection_policy.xsd">
    <content_models>
    <content_model dsid="ISLANDORACM" name="Islandora Collection Model ~ islandora:collectionCModel" namespace="islandora:1" pid="islandora:collectionCModel"/>
    </content_models>
    <search_terms>
    </search_terms>
    <staging_area></staging_area>
    <relationship>isMemberOfCollection</relationship>
</collection_policy>';

  if ($form_state['clicked_button']['#id'] == 'add_cm') {
    if (empty($policy->content)) {
      $collection_policy_datastream = $collection_object->constructDatastream('COLLECTION_POLICY', 'X');
      $collection_policy_datastream->setContentFromString($collection_policy);
      $collection_policy_datastream->label = 'COLLECTION_POLICY';
      $collection_object->ingestDatastream($collection_policy_datastream);
      $policy = $collection_object->getDatastream(variable_get('Islandora_Collection_Policy_DSID', 'COLLECTION_POLICY'));
    }

    $cp_namespace = $form_state['values']['new_cp_namespace'];
    $cp_content_model = $form_state['values']['content_model_to_add'];

    $content_model_object = islandora_object_load($cp_content_model);
    $content_model_datastream = $content_model_object->getDatastream(variable_get('Islandora_Content_Model_DSID', 'ISLANDORACM'));

    $collection_policy = new CollectionPolicy($policy->content);
    $collection_policy->addContentModel(
      $content_model_object->id,
      $content_model_object->label,
      $cp_namespace
    );

    $new_collection_policy_datastream = $collection_object->getDatastream('COLLECTION_POLICY');
    $new_collection_policy_datastream->setContentFromString($collection_policy->getXML());
    $new_collection_policy_datastream->label = 'COLLECTION_POLICY';
    $collection_object->ingestDatastream($new_collection_policy_datastream);
    drupal_set_message(t("Collection model @model successfully added", array('@model' => $content_model_object->label)));
  }

  //remove content model from collection policy
  if ($form_state['clicked_button']['#id'] == 'remove_cm') {
    $candidates = $form_state['values']['content_models_to_remove'];
    $collection_policy = new CollectionPolicy($policy->content);
    $count = $collection_policy->removeContentModel($candidates);
    if ($count > 0) {
      $new_collection_policy_datastream = $policy;
      $new_collection_policy_datastream->setContentFromString($collection_policy->getXML());
      $new_collection_policy_datastream->label = 'COLLECTION_POLICY';
      $collection_object->ingestDatastream($new_collection_policy_datastream);
      drupal_set_message(format_plural($count, "1 model removed from the collection policy.", "@count models removed from the collection policy."));
    }
  }
}
