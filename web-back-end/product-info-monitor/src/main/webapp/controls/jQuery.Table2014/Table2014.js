// http://stackoverflow.com/questions/1117086/how-to-create-a-jquery-plugin-with-methods
// How to create a jQuery plugin with methods?

(function ($) {
    $.fn.table2014 = function (options) {

        //默认值
        var defaultVal = {
            tableTitle: '表格标题',
            hrLine: true,
            tableHeader: true,
            hoverColor: false,
            columns: [],
            onSortCol: null,
            data: null,
            dataUrl: null,
            parms: {},
            method: 'get',
            async: true,
            cache: false,
            dataType: 'json',
            onClick: null,
            onLoadingError: null,
            onDataLoaded: null,
            dataRowsPath: 'rows' // can be like this: a.b.rows
        };

        var $args = arguments;

        var $plugin = this;

        return this.each(function () {
            // Public methods
            // Usage: $('.tree').tree2014('expand', el);
            var methods = {
                requestServerData: function (parameters) {
                    var table = $(this).data('table2014');
                    if (table) {
                        table.processServerData(parameters);
                    }
                }
            }

            //Check for methods
            if (typeof options === 'string') {
                if ($.isFunction(methods[options])) {
                    // do some check and prepare
                    // apply传入的this对象很重要，在public method中通过this引用
                    methods[options].apply(this, Array.prototype.slice.call($args, 1));
                    //http://stackoverflow.com/questions/1986896/what-is-the-difference-between-call-and-apply
                }
                return;
            }

            var $elem = $(this);

            // Initialize
            var opt = $.extend(defaultVal, options);

            var rendTable = function () {
                $elem.html('');
                var table = $('<div class="table2014"></div>').appendTo($elem);

                // 标题
                if (opt.tableTitle) {
                    var title = $(
                        '<div class="table-title-container">                        \
                            <div class="table-title-icon table-icon-default"></div> \
                            <div class="table-title-text"></div>                    \
                        </div>'
                    ).appendTo(table);

                    title.find('.table-title-text').text(opt.tableTitle);
                }

                // 分割线
                if (opt.hrLine) {
                    $('<div class="hrline"></div>').appendTo(table);
                }

                var rowsContainer = $('<div class="table-content-container table-fixed"></div>')
                        .appendTo(table);

                // 表头
                if (opt.tableHeader) {
                    var header = '<div class="table-row table-header">';

                    $.each(opt.columns, function (i) {
                        var width = (this.width && this.width > 0) ? (this.width + 'px') : 'auto';
                        var align = this.align ? this.align : 'left';

                        header += '<div class="table-cell" data-colName="' + this.colName + '" style="width:' + width + ';" text-align="' + align + '"><a href="#">' + this.header + '</a></div>';

                    });

                    header += '</div>';
                    rowsContainer.append(header);
                }

                // 表体
                if (opt.data != null) {
                    var optData = 'opt.data';
                    if (opt.dataRowsPath != null && opt.dataRowsPath.length > 0) {
                        optData += '.' + opt.dataRowsPath;
                    }
                    var bodyData = eval(optData);
                    renderTableBody(bodyData, rowsContainer);
                } else if (opt.dataUrl != null) {
                    $elem.processServerData(opt.parms);
                } else {
                    alert('no data or dataUrl setted.');
                }
            }

            $elem.processServerData = function (parms) {
                var rowsContainer = $elem.find('div.table-content-container');

                loadServerData(parms, function (data) {
                    var ajaxData = 'data';
                    if (opt.dataRowsPath != null && opt.dataRowsPath.length > 0) {
                        ajaxData += '.' + opt.dataRowsPath;
                    }
                    var bodyData1 = eval(ajaxData);
                    renderTableBody(bodyData1, rowsContainer);
                });
            }

            $elem.ajaxRequest = null;
            $elem.counter = 0;
            var loadServerData = function (parms, onSuccess) {
                // 防止并行重复调用 // 是否起效暂时还无法确认
                if ($elem.ajaxRequest) {
                    $elem.ajaxRequest.abort();
                }
                var ajaxOptions = {
                    //type: opt.method,
                    url: opt.dataUrl,
                    data: parms,
                    async: opt.async,
                    cache: opt.cache,
                    dataType: opt.dataType,
                    beforeSend: function () {
                        $elem.loadingUI();
                    },
                    success: function (data) {
                        $elem.ajaxRequest = null;
                        $('body').loadingUI('hide');
                        // 供用户在生成表格前修改原始数据
                        if (opt.onDataLoaded && $.isFunction(opt.onDataLoaded)) {
                            opt.onDataLoaded(data);
                        }
                        onSuccess(data);
                    },
                    error: function (xhr, e) {
                        $('body').loadingUI('hide');
                        if (e == 'abort') {
                            return;
                        }
                        if (opt.onLoadingError && $.isFunction(opt.onLoadingError)) {
                            opt.onLoadingError(xhr, e);
                        }
                    }
                };
                if (opt.contentType) ajaxOptions.contentType = opt.contentType;
                if (opt.dataType === 'jsonp') {
                    ajaxOptions.jsonpCallback = 'callBackFake'; // 经试验，jQuery能自动在success事件中对jsonp数据进行处理，无需从callback获得事件
                }

                $elem.ajaxRequest = $.ajax(ajaxOptions);
                $elem.counter++;
                // for debug:
                window.status = 'loadServerData: ' + $elem.counter;
            }

            // 因性能问题，此处改由拼字符串生成
            // 经对比，直接用字符串拼接好后再Append，性能高很多
            var renderTableBody = function (bodyData, rowsContainer) {
                $('div.table-content', rowsContainer).remove();

                $elem.data('bodyData', bodyData);

                var tableBodyHtml = '';

                $.each(bodyData, function (r) {
                    var rowData = this;

                    tableBodyHtml += '<div class="table-row table-content" data-rowIndex="{0}">'.format(r);

                    $.each(opt.columns, function (c) {
                        var col = this;

                        var cellHtml = '';
                        // 单元格内容
                        if (col.render && $.isFunction(col.render)) {
                            cellHtml = col.render(rowData, r + 1);
                        } else if (col.href) {
                            cellHtml = '<a class="linktext" href="{0}" title="{1}" "{2}">{3}</a>'.format(
                                rowData[col.href],
                                rowData[col.colName] || '',
                                col.target ? 'target="' + col.target + '"' : '',
                                rowData[col.colName] || ''
                            );
                        } else {
                            cellHtml = rowData[col.colName] || '';
                        }


                        // 表格风格，宽度和对齐
                        var cellStyle = '';
                        if (!opt.tableHeader && r == 0) { // 只有没有表头时，对第一行进行格式化
                            cellStyle += 'style="';
                            if (col.width && col.width > 0) {
                                cellStyle += 'width:' + col.width + 'px;';
                            } else {
                                cellStyle += 'width: auto;';
                            }
                            cellStyle += 'text-align: ' + (col.align || 'left') + ';';
                            cellStyle += '"';
                        }

                        tableBodyHtml += '<div class="table-cell" data-colIndex="{0}" {1}>{2}</div>'.format(c, cellStyle, cellHtml);


                    });
                    tableBodyHtml += '</div>'; // tableRow
                });

                rowsContainer.append(tableBodyHtml);

                // IE浏览器在某些情况下不会repaint，强制刷之
                // btw: IE11真变态，只有在docfile测试环境第二页才会发生这种情况
                // huangfucai, 2015-04-21
                var refresher = $('<div style="height: 1px; overflow: hidden;">_</div>').appendTo(rowsContainer);
                setTimeout(function () {
                    refresher.remove();
                }, 1);
                // end.
            }

            var bindEvent = function () {
                // 列超链接点击事件
                $elem.off('click', '.table-content .table-cell a').on('click', '.table-content .table-cell a', function (e) {
                    var colIndex = $(this).parents('.table-cell').attr('data-colIndex');
                    var rowIndex = $(this).parents('.table-row').attr('data-rowIndex');
                    var funOnClick = opt.columns[colIndex].onAnchorClick;
                    if (funOnClick) {
                        e.preventDefault();
                        funOnClick(e, $elem.data('bodyData')[rowIndex]);
                    }
                });

                // 列排序
                $elem.off('click', '.table-header .table-cell a').on('click', '.table-header .table-cell a', function (e) {
                    e.preventDefault();

                    if (opt.onSortCol && $.isFunction(opt.onSortCol)) {
                        var div = $(this).parents('.table-cell');

                        var colName = div.data('colName'.toLowerCase());
                        if (colName == null)
                            return;

                        var sortorSpan = div.find('span.sortor');
                        var sortAsc = (sortorSpan.length < 1) || (sortorSpan.hasClass('sortdesc'));
                        opt.onSortCol(colName, sortAsc);

                        if (sortorSpan.length < 1) {
                            div.append('<span class="sortor"></span>');
                        }

                        $elem.find('.table-header span.sortor').removeClass('sortasc').removeClass('sortdesc');

                        if (sortAsc) {
                            div.find('span.sortor').removeClass('sortdesc').addClass('sortasc');
                        } else {
                            div.find('span.sortor').removeClass('sortasc').addClass('sortdesc');
                        }
                    }
                });

                // 鼠标移动行高亮显示
                if (opt.hoverColor) {
                    $elem.off('mouseover', '.table-content').on('mouseover', '.table-content', function () {
                        $(this).addClass('row-hovered');
                    });

                    $elem.off('mouseout', '.table-content').on('mouseout', '.table-content', function () {
                        $(this).removeClass('row-hovered');
                    });
                     

                }
            }

            rendTable();
            bindEvent();
            $elem.data('table2014', $elem);
        });
    }

    if (!String.prototype.format) {
        String.prototype.format = function () {
            var args = arguments;
            return this.replace(/{(\d+)}/g, function (match, number) {
                return typeof args[number] != 'undefined' ? args[number] : match;
            });
        };
    }
})(jQuery);