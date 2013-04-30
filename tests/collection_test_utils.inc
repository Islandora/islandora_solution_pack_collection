<?php

/**
 * @file
 * Test class for extending classes that use islandora_basic_collection.
 */

class CollectionWebTestCase extends IslandoraWebTestCase {

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
    $this->drupalGet('islandora');
    $root = $this->url;
    $path = $root . '/manage/collection';
    $edit = array(
      'label' => $label,
      'pid' => $pid,
      'inherit_policy' => FALSE,
      'content_models[' . $model . ']' => TRUE,
    );
    $this->drupalPost($path, $edit, t('Create collection'), $options = array(), $headers = array(), $form_html_id = 'islandora-basic-collection-create-child-collection-form');
    $this->assertText($label, t('Created collection %label using the content model %model', array('%label' => $label, '%model' => $model)), 'Islandora');
  }

  /**
   * Searches for and deletes a collection from the root collection by label.
   *
   * @param string $label
   *   The label of the collection to be deleted
   *
   * @see IslandoraWebTestCase::createRootTestCollection()
   */
  public function deleteRootTestCollectionByLabel($label) {
    $collection = $this->openRootTestCollectionByLabel($label);
    if ($collection != FALSE) {
      $this->clickLink('Manage');
      $this->clickLink('Properties');
      $path = $this->url;
      $edit = array();
      $this->drupalPost($path, $edit, t('Delete Collection'));
      $path = $this->url;
      $this->drupalPost($path, $edit, t('Delete'));
      $this->pass(t('Deleted collection %label by searching the root collection', array('%label' => $label)), t('Islandora'));
    }
  }

  /**
   * Deletes a child collection from the root collection directly via PID.
   *
   * @param string $pid
   *   The PID of the collection to be deleted
   */
  public function deleteRootTestCollectionByPid($pid) {
    $path = 'islandora/object/' . $pid . '/manage/properties';
    $edit = array();
    $this->drupalPost($path, $edit, t('Delete Collection'));
    $path = $this->url;
    $this->drupalPost($path, $edit, t('Delete'));
    $this->assertPattern('/Deleted (\w+) children from/', t('Deleted collection directly via PID %pid', array('%pid' => $pid)), t('Islandora'));
  }

  /**
   * Searches through the root collection and opens a collection by label.
   *
   * Much slower than doing it by PID; use only when the PID is unknown.
   *
   * @param string $label
   *   The label of the collection
   * @param int $index
   *   The position of the link on the page
   */
  public function openRootTestCollectionByLabel($label, $index = 0) {
    $nexturls = $this->xpath('//a[normalize-space(text())=:label]', array(':label' => 'next >'));
    if (isset($nexturls[$index])) {
      while (isset($nexturls[$index])) {
        $urls = $this->xpath('//a[normalize-space(text())=:label]', array(':label' => $label));

        if (isset($urls[$index])) {
          $url_target = $this->getAbsoluteUrl($urls[$index]['href']);
        }

        if (isset($url_target)) {
          $this->assertTrue(isset($urls[$index]), t('Found collection %label in root collection', array('%label' => $label)), t('Islandora'));
          return $this->drupalGet($url_target);
        }
      }
      return FALSE;
    }
    else {
      $urls = $this->xpath('//a[normalize-space(text())=:label]', array(':label' => $label));

      if (isset($urls[$index])) {
        $url_target = $this->getAbsoluteUrl($urls[$index]['href']);
      }

      if (isset($url_target)) {
        $this->assertTrue(isset($urls[$index]), t('Found collection %label in root collection', array('%label' => $label)), t('Islandora'));
        return $this->drupalGet($url_target);
      }
    }
    return FALSE;
  }

}
