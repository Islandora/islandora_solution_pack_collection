<?php

/**
 * @file
 *
 * All collection policy management functionality.
 */

/**
 *
 * @param array $form
 * @param array $form_state
 * @param fedoraObject $collection_object
 * @param url $url
 * @return array
 */
function collection_policy_management($form, &$form_state, $collection_object, $url) {
  $pid = $collection_object->id;
  $table = islandora_collection_policy_table($pid);
  $alink_options = array(
    'attributes' => array('target' => '_blank'),
    'html' => TRUE,
  );
  $form = array();

  $form['link'] = array(
    '#type' => 'item',
    '#markup' => l(t("Return to Collection Manager"), $url),
  );

  $form['page_title'] = array(
    '#type' => 'item',
    '#markup' => '<h1>' . t('Manage Collection Policy') . '</h1>',
  );

  $form['help_link'] = array(
    '#type' => 'item',
    '#markup' => l(t('About Collection Policies'), 'https://wiki.duraspace.org/display/ISLANDORA112/How+to+Manage+Collection+Policies', $alink_options),
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['reorder_pages']['table'] = array(
    'table' => $table,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Update Collection Policy'),
  );
  return $form;
}

/**
 * Submit
 * @param array $form
 * @param array $form_state
 */
function collection_policy_management_submit($form, &$form_state) {
  $pid = $form_state['values']['pid'];
  $fedora_object = islandora_object_load($pid);
  $chosen = array_filter($form_state['values']['table']['selections']);
  $collection_policy = get_blank_collection_policy();
  $collection_policy_xml = simplexml_load_string($collection_policy);
  foreach ($form_state['values']['table']['rows'] as $candidate) {
    if (in_array($candidate['identifier'], $chosen)) {
      $cm_object = islandora_object_load($candidate['identifier']);
      $node = $collection_policy_xml->content_models->addChild('content_model');
      $node->addAttribute('dsid', 'ISLANDORACM');
      // prompt removed for this iteration
      $node->addAttribute('name', $cm_object->label);
      $node->addAttribute('namespace', $candidate['namespace'] . ':1');
      $node->addAttribute('pid', $candidate['identifier']);
    }
  }
  $fedora_object['COLLECTION_POLICY']->content = $collection_policy_xml->saveXML();
}

/**
 * Returns a formatted table listing all members of the collection
 * defined by the $pid parameter
 * @param string $collection_pid
 * @return array
 */
function islandora_collection_policy_table($pid) {
  $object = islandora_object_load($pid);
  $collection_policy = $object['COLLECTION_POLICY'];
  $content = $collection_policy->content;

  $xml = new DOMDocument();
  $xml->loadXML($content);
  $content_models = $xml->getElementsByTagName('content_models')->item(0)->getElementsByTagName('content_model');
  $current_vals = array();
  foreach ($content_models as $cm) {
    $pid = $cm->getAttribute('pid');
    $namespace = $cm->getAttribute('pid');
    $current_vals[$pid]['namespace'] = substr($cm->getAttribute('namespace'), 0, strpos($cm->getAttribute('namespace'), ":"));
    $current_vals[$pid]['prompt'] = $cm->getAttribute('name');
    ;
  }

  $is_restricted = variable_get('fedora_namespace_restriction_enforced', FALSE);
  if ($is_restricted) {
    $permitted = array();
    $allowed_namespaces = explode(": ", variable_get('fedora_pids_allowed', 'default: demo: changeme: islandora: books: newspapers:'));
    foreach ($allowed_namespaces as $allowed_namespace) {
      $allowed_namespace = str_replace(':', '', $allowed_namespace);
      $permitted[$allowed_namespace] = trim($allowed_namespace);
    }
  }

  $keys = array();
  $allowed_content_models = array();
  $default_namespace = substr($pid, 0, strpos($pid, ":"));
  // $collection_policy = CollectionPolicy::loadFromCollection($pid);
  $content_models = get_content_models_as_option_array();

  // results are returned with name appended to pid.  We strip the name out here
  foreach ($content_models as $pid => $string) {
    $string_parts = explode('~', $string);
    $content_models[$pid] = $string_parts[0];
    $keys[$pid] = '';
  }

  if ($collection_policy) {
    $supported_collection_models = array_keys($current_vals);
  }

  foreach ($supported_collection_models as $current) {
    $allowed_content_models[$current] = $current;
  }
  //move current content models to the top of the display
  foreach ($content_models as $content_model => $value) {
    if (in_array($content_model, $allowed_content_models)) {
      $content_models = array($content_model => $value) + $content_models;
    }
  }

  $table = array(
    '#header' => array(theme('table_select_header_cell'), t('Content Model Pid'), 'Namespace'),
    '#theme' => 'islandora_collection_policy_table',
    '#tree' => TRUE,
    'rows' => array(),
    'selections' => array(
      '#type' => 'checkboxes',
      '#options' => $keys,
      '#default_value' => $allowed_content_models,
    ),
  );
  $rows = &$table['rows'];
  if (empty($content_models)) {
    return;
  }
  foreach ($content_models as $pid => $string) {
    if ($pid == 'islandora:collectionCModel') {
      continue;
    }

    $namespace = isset($current_vals[$pid]['namespace']) ? $current_vals[$pid]['namespace'] : $default_namespace;
    $prompt = isset($current_vals[$pid]['prompt']) ? $current_vals[$pid]['prompt'] : t("New") . " $string";
    $cells = array();
    global $base_url;
    $markup = "<a href = '$base_url/islandora/object/$pid' target = '_blank'>$pid</a>";
    $cells['#pid'] = $pid;
    $cells['pid'] = array('#type' => 'item', '#markup' => $markup);

    if ($is_restricted) {
      $cells['namespace'] = array('#type' => 'select', '#options' => $permitted, '#default_value' => $namespace);
    }
    else {
      $cells['namespace'] = array('#type' => 'textfield', '#default_value' => $namespace);
    }

    $cells['identifier'] = array('#type' => 'hidden', '#value' => $pid);
    $rows[] = $cells;
  }

  return $table;
}

/**
 * themes the form table.
 *
 * @param array $element Drupal Form Element.
 * @return string
 */
function theme_islandora_collection_policy_table(array $element) {
  $element = array_shift($element);
  $rows = array();
  $children = element_children($element['rows']);
  foreach (element_children($element['rows']) as $child) {
    $setting = $element['rows'][$child];
    $pid = $setting['#pid'];

    $fields = array(
      drupal_render($element['selections'][$pid]) // First field is a checkbox
    );
    foreach (element_children($setting) as $property) {
      $field = $setting[$property];
      $fields[] = drupal_render($field);
    }
    $rows[] = array(
      'data' => $fields,
      'class' => isset($setting['#attributes']['class']) ? $setting['#attributes']['class'] : NULL
    );
  }
  $attributes = isset($element['#id']) ? array('id' => $element['#id']) : NULL;
  $variables = array(
    'header' => $element['#header'],
    'rows' => $rows,
    'attributes' => $attributes,
    'caption' => NULL,
    'colgroups' => NULL,
    'sticky' => NULL,
    'empty' => t("No data"),
  );
  return theme_table($variables);
}

/**
 * Gets blank Collection policy
 * @return string
 */
function get_blank_collection_policy() {
  return '<?xml version="1.0" encoding="UTF-8"?>
<collection_policy xmlns="http://www.islandora.ca" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" name="" xsi:schemaLocation="http://www.islandora.ca http://syn.lib.umanitoba.ca/collection_policy.xsd">
    <content_models>
    <content_model dsid="ISLANDORACM" name="Islandora Collection Model ~ islandora:collectionCModel" namespace="islandora:1" pid="islandora:collectionCModel"/>
    </content_models>
    <search_terms>
    </search_terms>
    <staging_area></staging_area>
    <relationship>isMemberOfCollection</relationship>
</collection_policy>';
}

