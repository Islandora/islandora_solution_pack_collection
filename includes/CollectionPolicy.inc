<?php

/**
 * @file
 *
 * This file defines a class for parsing the COLLECTION_POLICY xml document.
 * As defined here http://syn.lib.umanitoba.ca/collection_policy.xsd.
 */

/**
 * Collection Policy
 */
class CollectionPolicy {

  /**
   * The Collection Policy xml file.
   *
   * @var DOMDocument
   */
  protected $xml;

  /**
   * Creates a new CollectionPolicy object.
   *
   * @param string $xml
   *   The COLLECTION_POLICY xml in string form.
   */
  public function __construct($xml) {
    $this->xml = new DOMDocument();
    $this->xml->loadXML($xml);
    $path = drupal_get_path('module', 'islandora_basic_collection');
    if (!$this->xml->schemaValidate("$path/xml/collection_policy.xsd")) {
      throw new InvalidArgumentException('The given Collection Policy is not valid');
    }
  }

  /**
   * Gets the name of the Collection Policy.
   *
   * @return string
   *   The name of this Collection Policy.
   */
  function getName() {
    return $this->xml->getElementsByTagName('collection_policy')->item(0)->getAttribute('name');
  }

  /**
   * Gets the name of the relationship to use for members of this collection.
   *
   * @return string
   *   Gets the value of relationship if present, FALSE otherwise.
   */
  public function getRelationship() {
    return $this->xml->getElementsByTagName('relationship')->item(0)->nodeValue;
  }

  /**
   * Gets a list of ContentModel objects supported by this collection along with any other defaults used during ingestion.
   *
   * @return array
   *   An array describing each content model allowed in this collection each populated by key value pairs.
   *
   * @code
   *  array(
   *    'islandora:imageCModel' => array(
   *      'pid' => 'islandora:imageCModel',
   *      'name' => 'New Image',
   *      'namespace' => 'islandora'
   *    )
   *   )
   * @endcode
   */
  function getContentModels() {
    $ret = array();
    $models = $this->xml->getElementsByTagName('content_models')->item(0)->getElementsByTagName('content_model');
    foreach ($models as $model) {
      $pid = $model->getAttribute('pid');
      $matches = array();
      preg_match('/^([^:]*)/', $model->getAttribute('namespace'), $matches); // Clean up the namespace attribute as its wrong in most existing COLLECTION_POLICY datastreams.
      $ret[$pid] = array(
        'pid' => $pid,
        'name' => $model->getAttribute('name'),
        'namespace' => $matches[0]
      );
    }
    return $ret;
  }

}
