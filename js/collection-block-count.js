(function ($) {
  Drupal.behaviors.islandora_basic_collection_count_block = {
    attach: function (context, settings) {
      $.ajax({
        url: settings.islandora_basic_collection.count_block.callback,
        success: function (data, textStatus, jqXHR) {
          $('span#' + settings.islandora_basic_collection.count_block.id, context).html(data);
        }
      });
    }
  }
})(jQuery);
