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
 * @param AbstractObject $collection
 *   The collection object to purge the children from.
 * @param array $children
 *   The PID's of the children to delete, if NULL delete all child objects.
 *
 * @return array
 *   The batch definition.
 */
function islandora_basic_collection_delete_children_batch(AbstractObject $collection, $children = NULL) {
  $batch = array(
    'finished' => 'islandora_basic_collection_batch_finished',
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaining @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_basic_collection') . '/includes/batch.inc',
  );

  $message_parameters = array('@collection' => $collection->label);
  if (is_array($children)) {
    $child_count = count($children);
    $batch += array(
      'operations' => array(
        array(
          'islandora_basic_collection_delete_children_batch_operation',
          array($collection, $children),
        ),
      ),
      'title' => format_plural($child_count,
        'Deleting 1 child from @collection ...',
        'Deleting @count children from @collection ...', $message_parameters),
      'init_message' => format_plural($child_count,
        'Preparing to delete 1 child from @collection ...',
        'Preparing to delete @count children from @collection ...', $message_parameters),
    );
  }
  else {
    $batch += array(
      'operations' => array(
        array(
          'islandora_basic_collection_delete_all_children_batch_operation',
          array($collection),
        ),
      ),
      'title' => t('Deleting all children of @collection.', $message_parameters),
      'init_message' => t('Preparing to delete all children of @collection.', $message_parameters),
    );
  }

  return $batch;
}

/**
 * Deletes the given children from the given collection.
 *
 * Children are only deleted if the given collection is there only parent,
 * otherwise they are removed from the given collection.
 *
 * @param AbstractObject $collection
 *   The collection object to purge the children from.
 * @param array $children
 *   The PID's of the children to delete.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_delete_children_batch_operation(AbstractObject $collection, array $children, array &$context) {
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
    drupal_set_message(format_plural(count($children),
      'Deleted 1 child from <a href="@collection-url">@collection</a>.',
      'Deleted @count children from <a href="@collection-url">@collection</a>.',
      array(
        '@collection' => $collection->label,
        '@collection-url' => url("islandora/object/{$collection->id}"),
      )
    ));
  }
}

/**
 * Batch operation to delete ALL children from the given collection.
 *
 * @param AbstractObject $collection
 *   The collection object to purge the children from.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_delete_all_children_batch_operation(AbstractObject $collection, &$context) {
  $query_info = islandora_basic_collection_get_query_info(array(
    'object' => $collection,
  ));

  if (!isset($context['sandbox']['total_count'])) {
    $context['sandbox']['total_count'] = $collection->repository->ri->countQuery($query_info['query'], $query_info['type']);
    if ($context['sandbox']['total_count'] === 0) {
      drupal_set_message(t('No objects to delete in @collection.', array(
        '@collection' => $collection->label,
      )));
      $context['finished'] = 1;
      return;
    }
    $context['sandbox']['current'] = 0.0;
  }

  $results = islandora_basic_collection_get_member_objects($collection, 0, 10);
  if ($results) {
    list($count, $members) = $results;
    foreach ($members as $result) {
      $child = $result['object']['value'];
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
    }

    $context['sandbox']['current'] += $count;

    if ($count === 0) {
      $context['finished'] = 1;
    }
    else {
      $context['finished'] = $context['sandbox']['current'] / $context['sandbox']['total_count'];
    }
  }

  if (!$results || $context['finished'] == 1) {
    drupal_set_message(format_plural($context['sandbox']['current'],
      'Deleted 1 child from <a href="@collection-url">@collection</a>.',
      'Deleted @count children from <a href="@collection-url">@collection</a>.',
      array(
        '@collection' => $collection->label,
        '@collection-url' => url("islandora/object/{$collection->id}"),
      )
    ));
  }
}

/**
 * Migrate the given child objects from the source to destination collection.
 *
 * If no child objects are given its assumed all child objects are to be
 * migrated.
 *
 * @param AbstractObject $source
 *   The source collection object.
 * @param AbstractObject $destination
 *   The destination collection object.
 * @param array $children
 *   The PID's of the children to migrate, if NULL migrate all child objects.
 *
 * @return array
 *   The batch definition.
 */
function islandora_basic_collection_migrate_children_batch(AbstractObject $source, AbstractObject $destination, $children = NULL) {
  $children = is_array($children) ? $children : islandora_basic_collection_get_child_pids($collection);
  $parameters = array($source, $destination, $children);
  $migrate_operation = array('islandora_basic_collection_migrate_children_batch_operation', $parameters);
  $message_parameters = array('@source' => $source->label, '@destination' => $destination->label);
  return array(
    'operations' => array($migrate_operation),
    'finished' => 'islandora_basic_collection_batch_finished',
    'title' => t('Migrating children from @source to @destination ...', $message_parameters),
    'init_message' => t('Preparing to migrate children from @source to @destination ...', $message_parameters),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_basic_collection') . '/includes/batch.inc',
  );
}

/**
 * Moves the given children from the current collection to the new collection.
 *
 * @param AbstractObject $source
 *   The collection object the children are from.
 * @param AbstractObject $destination
 *   The destination collection.
 * @param array $children
 *   The PID's of the children to migrate.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_migrate_children_batch_operation(AbstractObject $source, AbstractObject $destination, array $children, array &$context) {
  $child = islandora_basic_collection_start_operation($children, $context);
  if ($child && $child = islandora_object_load($child)) {
    islandora_basic_collection_add_to_collection($child, $destination);
    islandora_basic_collection_remove_from_collection($child, $source);
  }
  if (islandora_basic_collection_end_operation($context)) {
    drupal_set_message(format_plural(count($children),
      'Migrated 1 child from <a href="@source-url">@source</a> to <a href="@destination-url">@destination</a>.',
      'Migrated @count children from <a href="@source-url">@source</a> to <a href="@destination-url">@destination</a>.',
      array(
        '@source' => $source->label,
        '@source-url' => url("islandora/object/{$source->id}"),
        '@destination' => $destination->label,
        '@destination-url' => url("islandora/object/{$destination->id}"),
      )
    ));
  }
}

/**
 * Share the given child objects from the source with destination collection.
 *
 * If no child objects are given its assumed all child objects are to be
 * shared.
 *
 * @param AbstractObject $source
 *   The source collection object.
 * @param AbstractObject $destination
 *   The destination collection object.
 * @param array $children
 *   The PID's of the children to share, if NULL share all child objects.
 *
 * @return array
 *   The batch definition.
 */
function islandora_basic_collection_share_children_batch(AbstractObject $source, AbstractObject $destination, $children = NULL) {
  $children = is_array($children) ? $children : islandora_basic_collection_get_child_pids($collection);
  $parameters = array($source, $destination, $children);
  $copy_operation = array('islandora_basic_collection_share_children_batch_operation', $parameters);
  $message_parameters = array('@source' => $source->label, '@destination' => $destination->label);
  return array(
    'operations' => array($copy_operation),
    'finished' => 'islandora_basic_collection_batch_finished',
    'title' => t('Sharing children from @source to @destination ...', $message_parameters),
    'init_message' => t('Preparing to share children from @source to @destination ...', $message_parameters),
    'progress_message' => t('Time elapsed: @elapsed <br/>Estimated time remaning @estimate.'),
    'error_message' => t('An error has occurred.'),
    'file' => drupal_get_path('module', 'islandora_basic_collection') . '/includes/batch.inc',
  );
}

/**
 * Shares the given children from the current collection to the new collection.
 *
 * @param AbstractObject $source
 *   The collection object the children are from.
 * @param AbstractObject $destination
 *   The destination collection.
 * @param array $children
 *   The PID's of the children to share.
 * @param array $context
 *   The batch context.
 */
function islandora_basic_collection_share_children_batch_operation(AbstractObject $source, AbstractObject $destination, array $children, array &$context) {
  $child = islandora_basic_collection_start_operation($children, $context);
  if ($child && $child = islandora_object_load($child)) {
    islandora_basic_collection_add_to_collection($child, $destination);
  }
  if (islandora_basic_collection_end_operation($context)) {
    drupal_set_message(format_plural(count($children),
      'Shared 1 child from <a href="@source-url">@source</a> to <a href="@destination-url">@destination</a>.',
      'Shared @count children from <a href="@source-url">@source</a> to <a href="@destination-url">@destination</a>.',
      array(
        '@source' => $source->label,
        '@source-url' => url("islandora/object/{$source->id}"),
        '@destination' => $destination->label,
        '@destination-url' => url("islandora/object/{$destination->id}"),
      )
    ));
  }
}
