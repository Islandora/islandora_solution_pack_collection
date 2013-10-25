<?php

/**
 * @file
 * Handles the display and submission of this modules admin form.
 */

/**
 * Defines the admin settings form.
 *
 * @param array $form
 *   The Drupal form definition.
 * @param array $form_state
 *   The Drupal form state.
 *
 * @return array
 *   The Drupal form definition.
 */
function islandora_basic_collection_admin(array $form, array &$form_state) {
  $form = array(
    'islandora_basic_collection_page_size' => array(
      '#type' => 'textfield',
      '#title' => t('Default collection objects per page'),
      '#default_value' => variable_get('islandora_basic_collection_page_size', '10'),
      '#description' => t('The default number of objects to show in a collection view.'),
    ),
    'islandora_basic_collection_disable_collection_policy_delete' => array(
      '#type' => 'checkbox',
      '#title' => t('Disable deleting the collection policy'),
      '#default_value' => variable_get('islandora_basic_collection_disable_collection_policy_delete', TRUE),
      '#description' => t("Disables the 'delete' link for the COLLECTION_POLICY datastream."),
    ),
    'islandora_basic_collection_disable_count_object' => array(
      '#type' => 'checkbox',
      '#title' => t('Disable object count query in collection view'),
      '#default_value' => variable_get('islandora_basic_collection_disable_count_object', FALSE),
      '#description' => t("Disabling the object count query can improve performance when viewing large collections"),
    ),
    'islandora_basic_collection_default_view' => array(
      '#type' => 'select',
      '#title' => t('Default collection view style.'),
      '#default_value' => variable_get('islandora_basic_collection_default_view', 'grid'),
      '#options' => array(
        'list' => t('List'),
        'grid' => t('Grid'),
      ),
    ),
  );
  return system_settings_form($form);
}
