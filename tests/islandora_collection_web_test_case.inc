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
    $object = islandora_object_load($pid);
    $this->assertTrue(is_object($object), t('Created collection %label using the content model %model', array('%label' => $label, '%model' => $model)), 'Islandora');
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
    $object = islandora_object_load($pid);
    if (is_object($object)) {
      $this->fail(t("Tuque reports that the object %pid still exists after attempting to purge it through the interface. This test will attempt to purge the collection directly through tuque.", array('%pid' => $pid)), 'Islandora');
      islandora_delete_object($object);
      if (isset($object->label)) {
        $label = $object->label;
      }
      else {
        $label = "unknown";
      }
      $this->assertFalse(is_object($object), t("Tuque reports that the object %pid still exists, despite an attempt to purge it directly. If this is correct, you may need to remove this collection manually (if verbose messaging is enabled in the testing module, you can check the last verbose message before this error to see whether or not it was actually deleted). The collection's label is %label.", array('%pid' => $pid, '%label' => $label)), 'Islandora');
    }
    else {
      $this->pass(t("Collection object %pid successfully deleted.", array('%pid' => $pid)), 'Islandora');
    }
  }

}
