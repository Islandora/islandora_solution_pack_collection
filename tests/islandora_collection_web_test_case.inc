<?php

/**
 * @file
 * Test class for extending classes that use islandora_basic_collection.
 */

class IslandoraCollectionWebTestCase extends IslandoraWebTestCase {

  /**
   * Randomly generates a child collection in the top-level collection.
   *
   * Use in lieu of throwing test objects into a standard collection.
   *
   * @param string $label
   *   The label that should be assigned to the collection
   * @param string $model
   *   The content model the collection should use
   * @param string $pid
   *   The PID that should be assigned to the collection
   *
   * @see IslandoraWebTestCase::deleteRootTestCollection()
   */
  public function createTestCollection($label, $model, $pid = '') {
    $this->drupalGet('islandora');
    $root = $this->url;
    $path = $root . '/manage/overview/ingest';
    if (!isset($pid)) {
      $pid_suffix = randomName(16);
      $pid = 'islandora:' . $pid_suffix;
    }
    $edit = array(
      'label' => $label,
      'pid' => $pid,
      'inherit_policy' => FALSE,
      'content_models[' . $model . ']' => TRUE,
    );
    $this->drupalPost($path, $edit, t('Ingest'));
    $this->assertTrue(islandora_object_load($pid), t('Created collection %label using the content model %model', array('%label' => $label, '%model' => $model)), 'Islandora');
  }

  /**
   * Deletes a child collection from the root collection directly via PID.
   *
   * @param string $pid
   *   The PID of the collection to be deleted
   */
  public function deleteTestCollection($pid) {
    $path = 'islandora/object/' . $pid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, t('Delete Collection'));
    $path = $this->url;
    $this->drupalPost($path, $edit, t('Delete'));
    $this->assertFalse(islandora_object_load($pid), t("Collection deleted directly via pid %pid.", array('%pid' => $pid)), 'Islandora');
  }
}
