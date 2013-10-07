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
   * @param string|array $models
   *   The collection's content models; also accepts a single-model string
   * @param string $pid
   *   The PID that should be assigned to the collection
   *
   * @see IslandoraWebTestCase::deleteRootTestCollection()
   */
  public function createTestCollection($label, $models, $pid = '') {
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
    );

    // Figure out enabling content models.
    $models = (array) $models;
    foreach ($models as $model) {
      $edit['content_models[' . $model . ']'] = TRUE;
    }

    $this->drupalPost($path, $edit, t('Ingest'));
    $object = islandora_object_load($pid);
    $model_list = implode(',', $models);
    $this->assertTrue(is_object($object), t('Created collection %label using the content model(s) %model_list', array('%label' => $label, '%model_list' => $model_list)), 'Islandora');
  }

  /**
   * Deletes a child collection from the root collection directly via PID.
   *
   * @param string $pid
   *   The PID of the collection to be deleted
   */
  public function deleteTestCollection($pid) {
    $path = 'islandora/object/' . $pid . '/delete';
    $this->drupalPost($path, array(), t('Delete'));
    $tuque = islandora_get_tuque_connection();
    $tuque->cache->resetCache();
    $object = islandora_object_load($pid);
    if (is_object($object)) {
      $this->fail(t("Tuque reports that the collection object %pid still exists after attempting to purge it through the interface. This test will attempt to purge the collection directly through tuque.", array('%pid' => $pid)), 'Islandora');
      try {
        islandora_delete_object($object);
        islandora_object_load($pid);
      }
      catch (Exception $e) {
        $this->fail(t("Tuque reports that it was unable to purge the collection %pid directly. This likely means that it does not actually exist. If verbose messaging is enabled in the testing module, you can check the last verbose message before this error to see whether or not it was actually purged.", array('%pid' => $pid)), 'Islandora');
      }
      $this->fail(t("Collection object %pid reports that deletion via tuque has failed. It may need to be deleted manually.", array('%pid' => $pid)), 'Islandora');
    }
    else {
      $this->pass(t("Collection object %pid successfully deleted.", array('%pid' => $pid)), 'Islandora');
    }
  }

}
