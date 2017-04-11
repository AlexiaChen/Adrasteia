// This splitter require jquery ui draggable.

(function ($) {
    var methods = {
        init: function (options) {
            var settings = $.extend({
                leftPaneWidth: 300,
                leftPaneMinWidth: 100,
                leftPaneMaxWidth: 1000
            }, options);

            return this.each(function () {

                var $this = $(this);
                var dragBar = $('div.splitv-dragbar', $this);

                var leftPane = $('div[data-layout=left]', $this);
                var centerPane = $('div[data-layout=center]', $this);
                var dragbarCollapse = $('div.dragbar-collapse', $this);

                // get additional parameters
                if (leftPane.data('layout-width')) {
                    settings.leftPaneWidth = leftPane.data('layout-width');
                }
                if (leftPane.data('layout-minWidth')) {
                    settings.leftPaneMinWidth = leftPane.data('layout-minWidth');
                }
                if (leftPane.data('layout-maxWidth')) {
                    settings.leftPaneMaxWidth = leftPane.data('layout-maxWidth');
                }

                var setSplittorPosition = function(left){
                    dragBar.css('left', left);
                    leftPane.width(left - (leftPane.outerWidth(true) - leftPane.width())); // $.width() = $.outerWidth() - margin - border - padding
                    centerPane.css('left', left + dragBar.outerWidth());
                }

                setSplittorPosition(settings.leftPaneWidth);

                dragBar.draggable({
                    axis: 'x',
                    helper: 'clone',
                    iframeFix: true,
                    opacity: 0.75,
                    stop: function (e, ui) {
                        var left = ui.position.left;
                        setSplittorPosition(left);
                        dragbarCollapse.removeClass('invert');
                        $(window).trigger('resize');
                    },
                    drag: function (event, ui) {
                        // restrict drag distance
                        ui.position.left = ui.position.left < settings.leftPaneMinWidth ? settings.leftPaneMinWidth : ui.position.left;
                        ui.position.left = ui.position.left > settings.leftPaneMaxWidth ? settings.leftPaneMaxWidth : ui.position.left;
                    }
                });

                dragbarCollapse.off('click').on('click', null,function (e) {
                    dragbarCollapse.toggleClass('invert');
                    var left = 0;
                    if (dragbarCollapse.hasClass('invert')) {
                        dragBar.data('old-left', dragBar.position().left);
                    } else {
                        left = dragBar.data('old-left');
                        if (left == null || left < settings.leftPaneWidth)
                            left = settings.leftPaneWidth;
                    }
                    dragBar.css('left', left);
                    leftPane.width(left - (leftPane.outerWidth(true) - leftPane.width()));
                    centerPane.css('left', left + dragBar.outerWidth());
                });
            });
        },
        destroy: function () {
        }
    };

    $.fn.splitter = function (method) {
        if (methods[method]) {
            return methods[method].apply(this, Array.prototype.slice.call(arguments, 1));
        } else if (typeof method === 'object' || !method) {
            return methods.init.apply(this, arguments);
        } else {
            $.error('Method' + method + 'does not exist on jQuery.splitter');
        }
    };

    $("div[data-layout=splitv]").splitter();

})(jQuery);
