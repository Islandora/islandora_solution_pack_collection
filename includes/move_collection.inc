<?php

/**
 * @file
 * MoveCollection.inc
 */

/**
 * Create object migration form for the collection manager
 *
 * @param array $form_state
 * @param string $pid
 *
 * @return string
 */
function islandora_collection_migrate_form($form, &$form_state, $object, $url) {
  module_load_include('inc', 'islandora_basic_collection', 'collection_manager_table');
  $results = islandora_basic_collection_get_objects($object, 1, 25);
  $total_count = count($results);

  $offset = (empty($_GET['page'])) ? 0 : $_GET['page'];
  pager_default_initialize($total_count, 25);
  $pager = theme('pager', array('quantity' => 10));
  $potential_collections = get_collections_as_option_array();
  $table = islandora_collection_table($object, $offset);

  if (!$table) {
    $form['no_objects'] = array(
      '#type' => 'item',
      '#title' => t('No objects found in this collection'),
    );
    return $form;
  }
  $form = array();

  $form['page_title'] = array(
    '#type' => 'item',
    '#markup' => '<h1>' . t('Manage Membership of Objects') . '</h1>',
  );

  $form['migrate']['link'] = array(
    '#type' => 'item',
    '#markup' => l(t("Return to Collection Manager"), $url),
  );
  $form['migrate']['pager'] = array(
    '#type' => 'item',
    '#markup' => $pager,
  );

  $form['migrate'] ['action_select'] = array(
    '#title' => t('Choose action'),
    '#type' => 'radios',
    '#options' => array('add' => 'Add selected objects to another collection', 'move' => 'Move selected objects to another collection'),
    '#default_value' => 'add',
    '#description' => t('Objects can belong to one or many collections.')
  );

  $form['migrate']['titlebox'] = array(
    '#type' => 'item',
    '#title' => t("Move objects from @collection_pid", array('@collection_pid' => $object->id)),
  );

  $form['migrate']['new_collection'] = array(
    '#title' => t('New collection'),
    '#description' => t("All content will be migrated from @pid to the selected collection", array('@pid' => $object->id)),
    '#type' => 'select',
    '#options' => $potential_collections,
  );

  $form['migrate']['table'] = $table;

  $form['current'] = array(
    '#type' => 'hidden',
    '#value' => $object->id,
  );

  $form['migrate']['message'] = array(
    '#type' => 'item',
    '#value' => '',
  );

  $form['migrate']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Migrate selected objects'),
  );
  return $form;
}

/**
 * Submit handler for object migration form
 *
 * @param array $form
 * @param array $form_state
 */
function islandora_collection_migrate_form_submit($form, &$form_state) {
  $pids = array_filter($form_state['values']['table']);
  $new_collection = $form_state['values']['new_collection'];
  $current = $form_state['values']['current'];
  $action = $form_state['values']['action_select'];
  foreach ($pids as $pid) {
    $fedora_object = islandora_object_load($pid);
    $fedora_object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $new_collection);
    $fedora_object->relationships->add(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $new_collection);
    if ($action == 'move') {
      $fedora_object->relationships->remove(FEDORA_RELS_EXT_URI, 'isMemberOfCollection', $current);
    }
  }
}
