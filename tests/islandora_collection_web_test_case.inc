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
  public function createRootTestCollection($label, $model, $pid = '') {
    $current_user = $this->loggedInUser;
    $user = $this->drupalCreateUser(array(
      'view fedora repository objects',
      'create child collection',
      'ingest fedora objects',
    ));
    $this->drupalLogin($user);

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

    if ($current_user) {
      $this->drupalLogin($current_user);
    }
    else {
      $this->drupalLogout();
    }
  }

  /**
   * Deletes a child collection from the root collection directly via PID.
   *
   * @param string $pid
   *   The PID of the collection to be deleted
   */
  public function deleteRootTestCollectionByPid($pid) {
    $current_user = $this->loggedInUser;
    $user = $this->drupalCreateUser(array('manage object properties', 'delete fedora objects and datastreams'));

    $this->drupalLogin($user);

    $path = 'islandora/object/' . $pid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, t('Delete Collection'));
    $path = $this->url;
    $this->drupalPost($path, $edit, t('Delete'));
    $this->assertPattern('/Deleted (\w+) children from/', t('Deleted collection directly via PID %pid', array('%pid' => $pid)), t('Islandora'));

    if ($current_user) {
      $this->drupalLogin($current_user);
    }
    else {
      $this->drupalLogout();
    }
  }

 /**
   * Runs a series of assertLink() checks on an object's datastreams.
   *
   * @param string $objectpid
   *   The PID of the object to be passed to the GET request.
   * @param array $datastreams
   *   An array containing the list of datastream links to assert.
   *
   * @see DrupalWebTestCase::assertLink()
   */
  public function assertDatastreams($objectpid, array $datastreams) {
    $path = 'islandora/object/' . $objectpid . '/manage/datastreams';
    $this->drupalGet($path);
    foreach ($datastreams as $datastream) {
      $this->assertLink($datastream, 0, t('%datastream datastream created', array('%datastream' => $datastream)), 'Islandora');
      }
    }

  /**
   * Deletes a single object specified by its PID and identified by its label.
   *
   * @param string $objectpid
   *   The PID of the object to be deleted.
   * @param string $label
   *   The PID's title; used to assertNoText() after deletion.
   */
  public function deleteTestObject($objectpid, $label, $delete = 'Delete') {
    $path = 'islandora/object/' . $objectpid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, $delete);
    $path = $this->url;
    $this->drupalPost($path, $edit, t('Delete'));
    $this->assertNoText($label, t('Object %label successfully removed', array('%label' => $label)));
   }
}
