<?php

/**
 * @file
 *
 * Handles the display and submission of this modules admin form.
 */

/**
 * Handles the display of this modules admin form.
 *
 * @return array
 *   A Drupal form definition.
 */
function islandora_basic_collection_admin() {
  $form = array();
  $form['islandora_basic_collection_page_size'] = array(
    '#type' => 'textfield',
    '#title' => t('The default number of objects to show for a collection'),
    '#default_value' => variable_get('islandora_basic_collection_page_size', '10'),
    '#description' => t('The default number of object to show in a collection view.'),
    '#weight' => -10
  );
  $form['islandora_basic_collection_disable_collection_policy_delete'] = array(
    '#type' => 'checkbox',
    '#title' => t('Disable deleting the collection policy'),
    '#default_value' => variable_get('islandora_basic_collection_disable_collection_policy_delete', TRUE),
    '#description' => t('This will disable the delete link for the COLLECTION_POLICY in the manage tab.'),
    '#weight' => -10
  );
  $form['islandora_basic_collection_default_view'] = array(
    '#type' => 'select',
    '#title' => t('Select the default collection view style.'),
    '#default_value' => variable_get('islandora_basic_collection_default_view', 'grid'),
    '#options' => array(
      'list' => t('List'),
      'grid' => t('Grid'),
    ),
    '#weight' => -10
  );

  return system_settings_form($form);
}
