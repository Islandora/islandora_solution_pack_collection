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
    $path = $root . '/manage/collection';
    $edit = array(
      'label' => $label,
      'pid' => $pid,
      'inherit_policy' => FALSE,
      'content_models[' . $model . ']' => TRUE,
    );
    $this->drupalPost($path, $edit, t('Create collection'), array(), array(), 'islandora-basic-collection-create-child-collection-form');
    $this->assertText($label, t('Created collection %label using the content model %model', array('%label' => $label, '%model' => $model)), 'Islandora');
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
    $this->assertPattern('/Deleted (\w+) children from/', t('Deleted collection directly via PID %pid', array('%pid' => $pid)), t('Islandora'));
  }

  /**
   * Asserts that an image can be created via the string output of a datastream.
   *
   * @param string $object
   *   The PID of the object
   * @param array $datastreams
   *   An array of strings containing datastream names
   */
  public function assertImageDatastreams($object, array $datastreams) {
    $this->drupalGet('islandora/object/' . $object);
    $base_url = $this->url;
    foreach ($datastreams as $datastream) {
      if (isset($object[$datastream])) {
        $image = $object[$datastream]->content;
        if (imagecreatefromstring($image)) {
          $this->pass("{$datastream} from PID {$object->id} is a valid image.", 'Islandora');
        }
        else {
          $this->fail("{$datastream} from PID {$object->id} is not a valid image.", 'Islandora');
        }
      }
    }
  }

  /**
   * Asserts that an array of image datastreams contain a provided imagetype.
   *
   * Indended to be used for image formats that can't be created with
   * imagecreatefromstring(). Less accurate than assertImageDatastreams, but
   * more compatible.
   *
   * @param string $object
   *   The PID of the object
   * @param array $datastreams
   *   An array containing pairs of datastream names and imagetypes.
   */
  public function assertImageDatastreamType($object, array $datastreams) {
    $this->drupalGet('islandora/object/' . $object);
    $base_url = $this->url;
    foreach ($datastreams as $datastream) {
      if (isset($object[$datastream[0]])) {
        $image = $object[$datastream[0]]->content;
        $path = '/tmp/' . $object->createdDate . '.' . $datastream[0];
        file_put_contents($path, $image);
        $type = exif_imagetype($path);
        if ($type == $datastream[1]) {
          $this->pass("{$datastream[0]} from PID {$object->id} matches expected type value ({$datastream[1]}).", 'Islandora');
        }
        else {
          $this->fail("{$datastream[0]} from PID {$object->id} contains unexpected type {$type}.", 'Islandora');
        }
      }
    }
  }

}
