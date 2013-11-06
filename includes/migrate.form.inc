<?php

/**
 * @file
 * Shares an object that is not a collection to an additional collection.
 */

function islandora_basic_collection_migrate_item_form($form, &$form_state, $islandora_object){
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('Migrate this item'),
  );
  $form['new_collection_name'] = array(
    '#autocomplete_path' => 'islandora/basic_collection/search_models',
    '#type' => 'textfield',
    '#title' => t('Name'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Share Object',
  );
  $form_state['basic_collection_share'] = $islandora_object->id ;
  return $form ;
}

function islandora_basic_collection_migrate_item_form_validate($form, &$form_state){
  $new_collection = islandora_object_load($form_state['values']['new_collection_name']);
  $collection_models = islandora_basic_collection_get_collection_content_models();
  $is_a_collection = FALSE ;
  if(is_object($new_collection)){
    $is_a_collection = (
      (count(array_intersect($collection_models, $new_collection->models)) > 0) &&
      isset($new_collection['COLLECTION_POLICY'])
    );
   }
  if(!$is_a_collection){
    form_set_error('new_collection_name', t('Not a valid collection'));
  }

 }

function islandora_basic_collection_migrate_item_form_submit($form, &$form_state){
  module_load_include('inc', 'islandora_basic_collection', 'includes/utilities');
  $object = islandora_object_load($form_state['basic_collection_share']);
  $new_collection = islandora_object_load($form_state['values']['new_collection_name']);

  if($object && $new_collection){
    islandora_basic_collection_add_to_collection($object, $new_collection);
    drupal_set_message(t('The object ' . $object->label . ' has been shared with ' . $new_collection->label));
  }

}
