<?php

/**
 * @file
 * Handles the ingestion of child objects that are not collections.
 */

/**
 * Manage action that for ingestion of an object into the given collection.
 *
 * @param FedoraObject $object
 *   The collection to ingest into.
 *
 * @return array
 *   The ingest form.
 */
function islandora_basic_collection_ingest_action(FedoraObject $object) {
  if (($configuration = islandora_basic_collection_get_ingest_configuration($object)) !== FALSE) {
    module_load_include('inc', 'islandora', 'includes/ingest.form');
    return drupal_get_form('islandora_ingest_form', $configuration);
  }
  drupal_not_found();
}

/**
 * Generates an ingest configuration from the given collection object.
 *
 * @see islandora_ingest_form
 *
 * @param FedoraObject $object
 *   A collection object to generate the configuration from.
 *
 * @return array
 *   An ingest configuration array as defined by islandora_ingest_form() if
 *   possible FALSE otherwise.
 */
function islandora_basic_collection_get_ingest_configuration(FedoraObject $object) {
  if (in_array('islandora:collectionCModel', $object->models) && isset($object['COLLECTION_POLICY'])) {
    $policy = new CollectionPolicy($object['COLLECTION_POLICY']->content);
    return array(
      'collections' => array($object->id),
      'models' => array_keys($policy->getContentModels()),
    );
  }
  return FALSE;
}

/**
 * Alter the ingest form to handle a change in the selected content model.
 *
 * The collection needs to add some additional  information to the ingest object
 * based on what form association was used to select the metadata form. Namely
 * it needs to provide a default namespace, and collection to be ingested into.
 * To do this it appends its own submit handler on to the metadata step.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 * @param string $form_id
 *   The islandora_ingest_form form id.
 */
function islandora_basic_collection_form_islandora_ingest_form_alter(array &$form, array &$form_state, $form_id) {
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  $step = islandora_ingest_form_get_step($form_state);
  if (isset($shared_storage['collections'])) {
    $step = islandora_ingest_form_get_step($form_state);
    $metadata_form_step = isset($step['form_id']) && $step['form_id'] == 'xml_form_builder_ingest_form';
    $submit_handler_already_added = in_array('islandora_basic_collection_ingest_form_submit', $form['next']['#submit']);
    if ($metadata_form_step && !$submit_handler_already_added) {
      array_unshift($form['next']['#submit'], 'islandora_basic_collection_ingest_form_submit');
    }
  }
}

/**
 * Set the default namespace.
 *
 * @param array $form
 *   The Drupal form.
 * @param array $form_state
 *   The Drupal form state.
 */
function islandora_basic_collection_ingest_form_submit(array $form, array &$form_state) {
  module_load_include('inc', 'islandora', 'includes/utilities');
  $shared_storage = islandora_ingest_form_get_shared_storage($form_state);
  // Only support 1 for now.
  $collection_id = current($shared_storage['collections']);
  // Only support 1 for now.
  $object = islandora_ingest_form_get_object($form_state);
  $collection_object = islandora_object_load($collection_id);
  $policy = new CollectionPolicy($collection_object['COLLECTION_POLICY']->content);
  $models = $policy->getContentModels();
  $association_step_storage = islandora_ingest_form_get_step_storage($form_state, 'xml_form_builder_metadata_step');
  $association = $association_step_storage['association'];
  // Get the form association's content model.
  $model = $association['content_model'];
  $model = $models[$model];
  $namespace = islandora_get_namespace($model['namespace']);
  $object->id = $object->repository->api->m->getNextPid($namespace);
}
