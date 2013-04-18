<?php

/**
 * @file
 * Test class to be extended when running classes that use the islandora_basic_collection content model.
 */

class CollectionWebTestCase extends IslandoraWebTestCase {

/**
 * Creates a randomly generated child collection in the top-level collection through the testing interface. This should be used in lieu of throwing test objects into a standard collection, in case that collection has been deleted.
 *
 * @param $label
 *   The label that should be assigned to the collection
 * @param $model
 *   The content model the collection should use
 * @param $pid
 *   The PID that should be assigned to the collection (left blank by default, using a sequentially assigned number)
 * @see IslandoraWebTestCase::deleteRootTestCollection()
 */
function createRootTestCollection($label, $model, $pid = '') {
  $this->drupalGet('islandora');
  $root = $this->url;
  $path = $root . '/manage/collection';
  $edit = array('label' => $label, 'pid' => $pid, 'inherit_policy' => FALSE, 'content_models[' . $model . ']' => TRUE);
  $this->drupalPost($path, $edit, t('Create collection'), $options = array(), $headers = array(), $form_html_id = 'islandora-basic-collection-create-child-collection-form');
  $this->assertText($label, t('Created collection %label using the content model %model', array('%label' => $label, '%model' => $model)), 'Islandora');
}

/**
 * Deletes a child collection from the root collection by searching for a particular label.
 *
 * @param $label
 *   The label of the collection to be deleted
 * @see IslandoraWebTestCase::createRootTestCollection()
 */
function deleteRootTestCollectionByLabel($label) {
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
 * Deletes a child collection from the root collection directly via PID
 */
function deleteRootTestCollectionByPid($pid) {
  $path = 'islandora/object/' . $pid . '/manage/properties';
  $edit = array();
  $this->drupalPost($path, $edit, t('Delete Collection'));
  $path = $this->url;
  $this->drupalPost($path, $edit, t('Delete'));
  $this->assertPattern('/Deleted (\w+) children from/', t('Deleted collection directly via PID %pid', array('%pid' => $pid)), t('Islandora'));
}

/**
 * Searches through the pages inside the root collection and opens the one specified by its label. This is much slower than doing it directly by PID; use this only in tests where the PID is unknown.
 *   @param $label
 *     The label of the collection
 *
 */
function openRootTestCollectionByLabel($label, $index = 0) {
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
