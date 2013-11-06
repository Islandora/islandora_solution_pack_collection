<?php

/**
 * @file
 * Shares an object that is not a collection to an additional collection.
 */

function islandora_basic_collection_share_item_form($form, &$form_state){
  $form['description'] = array(
    '#type' => 'item',
    '#title' => t('Migrate single item'),
  );
  $form['name'] = array(
    '#autocomplete_path' => 'islandora/basic_collection/search_models',
    '#type' => 'textfield',
     '#title' => t('Name'),
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Share Object',
  );

  return $form ;
}

function islandora_basic_collection_share_item_form_submit($form, &$form_state){


}
