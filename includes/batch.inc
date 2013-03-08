<?php

/**
 * @file
 * This file contains all the batch operations for collection objects.
 */

require_once dirname(__FILE__) . '/utilities.inc';

/**
 * Starts the batch operation.
 *
 * @param array $operands
 *   All the objects to operate on.
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_basic_collection_start_operation(array $operands, array &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = count($operands);
  }
  return @$operands[$context['sandbox']['progress']];
}

/**
 * Ends the batch operation.
 *
 * @param array $context
 *   The context of this batch operation.
 */
function islandora_basic_collection_end_operation(array &$context) {
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] < $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
    return FALSE;
  }
  return TRUE;
}

/**
 * The batch 'finished' callback.
 */
function islandora_basic_collection_batch_finished($success, $results, $operations) {
  if (!$success) {
    // Generic Error Message.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array(
                 '%error_operation' => $error_operation[0],
                 '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

/**
 * Delete the given child objects within the collection.
 *
 * If no child objects are given its assumed all child objects are to be
 * deleted.
 *
 * @param FedoraObject $collection
 *   The collection object to purge the children from.
 * @param array $children
 *   The PID's of the children to delete, if NULL delete all child objects.
 *
 * @return array
 *   The batch definition.
 */
function islandora_basic_collection_delete_children_batch(FedoraObject $collection, $children = NULL) {
  $children = is_array($children) ? $children : islandora_basic_collection_get_child_pids($collection);
  $parameters = array($collection, $children);
  $delete_child_operation = array('islandora_basic_collection_delete_children_batch_operation', $parameters);
  $message_parameters = array('@collection' => $collection->label, '@count' => count($children));
  return array(
    'operations' => array($delete_child_operation),
    'finished' => 'islandora_basic_collection_batch_finished',
    'title' => t('Deleting @count children from @collection', $message_parameters),
    'init_message' => t('Preparing to delete @count children from @collection.', $message_parameters),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_basic_collection') . '/includes/batch.inc',
  );
}

/**
 * Deletes the given children from the given collection.
 *
 * Children are only deleted if the given collection is there only parent,
 * otherwise they are removed from the given collection.
 *
 * @param FedoraObject $collection
 *   The collection object to purge the children from.
 * @param array $children
 *   The PID's of the children to delete.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_delete_children_batch_operation(FedoraObject $collection, array $children, array &$context) {
  $child = islandora_basic_collection_start_operation($children, $context);
  if ($child && $child = islandora_object_load($child)) {
    $other_parents = islandora_basic_collection_get_other_parents($child, $collection);
    // If only one parent, delete the object.
    if (empty($other_parents)) {
      islandora_delete_object($child);
    }
    else {
      // If more than one parent, only remove parent relationship.
      islandora_basic_collection_remove_from_collection($child, $collection);
    }
  }
  if (islandora_basic_collection_end_operation($context)) {
    drupal_set_message(t('Deleted @count children from !collection.', array(
          '!collection' => l($collection->label, "islandora/object/{$collection->id}"),
          '@count' => count($children))));
  }
}

/**
 * Migrate the given child objects from the source to destination collection.
 *
 * If no child objects are given its assumed all child objects are to be
 * migrated.
 *
 * @param FedoraObject $source
 *   The source collection object.
 * @param FedoraObject $destination
 *   The destination collection object.
 * @param array $children
 *   The PID's of the children to migrate, if NULL migrate all child objects.
 *
 * @return array
 *   The batch definition.
 */
function islandora_basic_collection_migrate_children_batch(FedoraObject $source, FedoraObject $destination, $children = NULL) {
  $children = is_array($children) ? $children : islandora_basic_collection_get_child_pids($collection);
  $parameters = array($source, $destination, $children);
  $migrate_operation = array('islandora_basic_collection_migrate_children_batch_operation', $parameters);
  $message_parameters = array('@source' => $source->label, '@destination' => $destination->label);
  return array(
    'operations' => array($migrate_operation),
    'finished' => 'islandora_basic_collection_batch_finished',
    'title' => t('Migrating children from @source to @destination', $message_parameters),
    'init_message' => t('Preparing to migrate children from @source to @destination.', $message_parameters),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_basic_collection') . '/includes/batch.inc',
  );
}

/**
 * Moves the given children from the current collection to the new collection.
 *
 * @param FedoraObject $source
 *   The collection object the children are from.
 * @param FedoraObject $destination
 *   The destination collection.
 * @param array $children
 *   The PID's of the children to migrate.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_migrate_children_batch_operation(FedoraObject $source, FedoraObject $destination, array $children, array &$context) {
  $child = islandora_basic_collection_start_operation($children, $context);
  if ($child && $child = islandora_object_load($child)) {
    islandora_basic_collection_add_to_collection($child, $destination);
    islandora_basic_collection_remove_from_collection($child, $source);
  }
  if (islandora_basic_collection_end_operation($context)) {
    drupal_set_message(t('Migrated @count children from !source to !destination.', array(
          '!source' => l($source->label, "islandora/object/{$source->id}"),
          '!destination' => l($destination->label, "islandora/object/{$destination->id}"),
          '@count' => count($children))));
  }
}

/**
 * Share the given child objects from the source with destination collection.
 *
 * If no child objects are given its assumed all child objects are to be
 * shared.
 *
 * @param FedoraObject $source
 *   The source collection object.
 * @param FedoraObject $destination
 *   The destination collection object.
 * @param array $children
 *   The PID's of the children to share, if NULL share all child objects.
 *
 * @return array
 *   The batch definition.
 */
function islandora_basic_collection_share_children_batch(FedoraObject $source, FedoraObject $destination, $children = NULL) {
  $children = is_array($children) ? $children : islandora_basic_collection_get_child_pids($collection);
  $parameters = array($source, $destination, $children);
  $copy_operation = array('islandora_basic_collection_share_children_batch_operation', $parameters);
  $message_parameters = array('@source' => $source->label, '@destination' => $destination->label);
  return array(
    'operations' => array($copy_operation),
    'finished' => 'islandora_basic_collection_batch_finished',
    'title' => t('Sharing children from @source to @destination', $message_parameters),
    'init_message' => t('Preparing to share children from @source to @destination.', $message_parameters),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_basic_collection') . '/includes/batch.inc',
  );
}

/**
 * Shares the given children from the current collection to the new collection.
 *
 * @param FedoraObject $source
 *   The collection object the children are from.
 * @param FedoraObject $destination
 *   The destination collection.
 * @param array $children
 *   The PID's of the children to share.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_share_children_batch_operation(FedoraObject $source, FedoraObject $destination, array $children, array &$context) {
  $child = islandora_basic_collection_start_operation($children, $context);
  if ($child && $child = islandora_object_load($child)) {
    islandora_basic_collection_add_to_collection($child, $destination);
  }
  if (islandora_basic_collection_end_operation($context)) {
    drupal_set_message(t('Shared @count children from !source with !destination.', array(
          '!source' => l($source->label, "islandora/object/{$source->id}"),
          '!destination' => l($destination->label, "islandora/object/{$destination->id}"),
          '@count' => count($children))));
  }
}
