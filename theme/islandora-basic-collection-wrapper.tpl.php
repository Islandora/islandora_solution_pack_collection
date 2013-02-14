<?php

/**
 * @file
 * islandora-basic-collection-wrapper.tpl.php
 *
 * @TODO: needs documentation about file and variables
 */
?>

<div class="islandora-basic-collection-wrapper">
  <div class="islandora-basic-collection clearfix">
    <span class="islandora-basic-collection-display-switch">
     <?php print theme('links', array('links' => $view_links, 'attributes' => array('class' => array('links', 'inline'))));?>
    </span>
    <?php print $collection_pager; ?>
    <?php print $collection_content; ?>
    <?php print $collection_pager; ?>
  </div>
</div>
