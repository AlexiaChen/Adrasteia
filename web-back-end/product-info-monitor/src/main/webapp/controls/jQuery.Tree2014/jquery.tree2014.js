// http://stackoverflow.com/questions/1117086/how-to-create-a-jquery-plugin-with-methods
// How to create a jQuery plugin with methods?

(function ($) {
    $.fn.tree2014 = function (options) {

        //默认值
        var defaultVal = {
            width: 220,
            nodePadding: 16,
            idAlias: 'Id',
            pidAlias: 'Pid',
            nameAlias: 'Name',
            expandedAlias: 'Expanded',
            iconAlias: 'Icon',
            iconBase: 'images/',
            hrefAlias: 'Href',
            onNodeClick: null,
            onTreeRendered: null
        };

        var $args = arguments;

        return this.each(function () {
            // Public methods
            // Usage: $('.tree').tree2014('expand', el);
            var methods = {
                expand: function (nodeId) {
                    var node = $('.TreeNode[data-id=' + nodeId + ']', $(this));
                    var children = $(this).find('.children-container[data-pid=' + nodeId + ']');
                    if (children.length < 1)
                        return;
                    var visible = children.is(':visible');
                    if (!visible) {
                        children.show();
                        node.find('.ExpandFlagContainer').removeClass('FlagCollapsed').addClass('FlagExpanded');
                    }
                },
                collapse: function (nodeId) {
                    var node = $('.TreeNode[data-id=' + nodeId + ']', $(this));
                    var children = $(this).find('.children-container[data-pid=' + nodeId + ']');
                    if (children.length < 1)
                        return;
                    var visible = children.is(':visible');
                    if (visible) {
                        children.hide();
                        node.find('.ExpandFlagContainer').removeClass('FlagExpanded').addClass('FlagCollapsed');
                    }
                },
                select: function (nodeId) {
                    var node = $('.TreeNode[data-id=' + nodeId + ']', $(this));
                    $('.TreeNodeSelected').removeClass('TreeNodeSelected');
                    node.addClass('TreeNodeSelected');
                }
            }

            // Check for methods
            if (typeof options === 'string') {
                if ($.isFunction(methods[options])) {
                    // do some check and prepare
                    // apply传入的this对象很重要，在public method中通过this引用
                    methods[options].apply(this, Array.prototype.slice.call($args, 1));
                    //http://stackoverflow.com/questions/1986896/what-is-the-difference-between-call-and-apply
                }
                return;
            }

            // Initialize
            var opt = $.extend(defaultVal, options);

            var $elem = $(this);

            var arrayToTree = function (data, idAlias, pidAlias)  //将ID、ParentID这种数据格式转换为树格式
            {
                if (!data || !data.length) return [];
                var targetData = []; // 存储数据的容器(返回) 
                var records = {};
                var itemLength = data.length; // 数据集合的个数
                for (var i = 0; i < itemLength; i++) {
                    var o = data[i];
                    records[o[idAlias]] = o;
                }
                for (var i = 0; i < itemLength; i++) {
                    var currentData = data[i];
                    var parentData = records[currentData[pidAlias]];
                    if (!parentData) {
                        targetData.push(currentData);
                        continue;
                    }
                    parentData.children = parentData.children || [];
                    parentData.children.push(currentData);
                }
                return targetData;
            }

            var rendTreeNodes = function (container, nodes, level) {
                if (level == null)
                    level = 1;
                var htmlTreeNode = // {3}:1/2/3  {4}:Images/defaulticon.png
                '<div class="TreeNode"> \
                    <div class="ExpandFlagContainer FlagNone"></div> \
                    <img class="TreeNodeIcon" alt="" /> \
                    <span class="TreeNodeText TextOverFlowEllipsis" title="{1}" style="width:{0}px">{1}</span> \
                </div>';

                for (var j = 0; j < nodes.length; j++) {
                    var node = nodes[j];
                    var html = htmlTreeNode.format(
                        opt.width - 100,
                        node[opt.nameAlias] || ''
                        );
                    var newNode = $(html).appendTo(container);

                    newNode.attr('data-id', node[opt.idAlias]);
                    newNode.attr('data-pid', node[opt.pidAlias]);
                    newNode.attr('data-href', node[opt.hrefAlias]);
                    newNode.data('nodedata', node);

                    // 缩进
                    $('.ExpandFlagContainer', newNode).css('margin-left', level * opt.nodePadding);

                    // 图标
                    var iconName = opt.defaultIcon;
                    if (node[opt.iconAlias] != null)
                        iconName = node[opt.iconAlias];
                    $('.TreeNodeIcon', newNode).attr('src', opt.iconBase + iconName);

                    if (node.children && node.children.length > 0) {
                        // 展开标记
                        $('.ExpandFlagContainer', newNode).removeClass('FlagNone').addClass(node.Expanded ? 'FlagExpanded' : 'FlagCollapsed');

                        // 子项目
                        var newContainer = $('<div class="children-container"></div>').attr('data-pid', node[opt.idAlias]).appendTo(container);
                        rendTreeNodes(newContainer, node.children, level + 1);
                        if (!node.Expanded) {
                            newContainer.hide();
                        }
                    }
                }
            }

            var renderSingleTree = function (treeObj) {
                // TreeContainer
                var htmlTreeContainer = '<div class="TreeContainer" data-treeid="{0}" style="width: {1}px"></div>';
                htmlTreeContainer = htmlTreeContainer.format(treeObj.TreeId, opt.width);
                var treeContainer = $(htmlTreeContainer).appendTo($elem);

                // TreeTitle
                // {0}: images/treeicon.png; {1}: 中国华融; {2}:TreeTitleExpanded/TreeTitleCollapsed
                var htmlTreeTitle =
                '<div class="TreeTitle"> \
                    <span class="TreeTitleIcon"><img alt="" src="{0}" /></span> \
                    <span class="TreeTitleText">{1}</span> \
                    <span class="TreeExpandFlag {2}"> \
                    </span> \
                 </div>';

                htmlTreeTitle = htmlTreeTitle.format(
                    opt.iconBase + treeObj.TreeIcon,
                    treeObj.TreeTitle,
                    treeObj.Expanded ? 'TreeTitleExpanded' : 'TreeTitleCollapsed');
                var treeTtile = $(htmlTreeTitle).appendTo(treeContainer);

                // TreeContent
                var htmlTreeContent = '<div class="TreeContent" style="display: {0};"></div>';
                htmlTreeContent = htmlTreeContent.format(treeObj.Expanded ? 'block' : 'none');
                var treeContent = $(htmlTreeContent).appendTo(treeContainer);

                // TreeNodes
                var treeData = arrayToTree(treeObj.TreeNodes, opt.idAlias, opt.pidAlias);
                rendTreeNodes(treeContent, treeData);
            }

            var rendTrees = function (plainData) {
                $elem.html('');

                $elem.disableSelection();

                if ($.isArray(plainData)) {
                    for (var i = 0; i < plainData.length; i++) {
                        renderSingleTree(plainData[i]);
                    }
                } else if (typeof (plainData) === 'object') {
                    renderSingleTree(plainData);
                }

                if (opt.onTreeRendered && $.isFunction(opt.onTreeRendered)) {
                    opt.onTreeRendered();
                }
            }

            var bindEvent = function () {
                $elem.off('click', '.TreeTitle').on('click', '.TreeTitle', function (e) {
                    var visible = $(this).parent().find('.TreeContent').is(':visible');
                    if (visible) {
                        $(this).parent().find('.TreeContent').hide();
                        $(this).find('.TreeExpandFlag').removeClass('TreeTitleExpanded').addClass('TreeTitleCollapsed');
                    } else {
                        $(this).parent().find('.TreeContent').show();
                        $(this).find('.TreeExpandFlag').removeClass('TreeTitleCollapsed').addClass('TreeTitleExpanded');
                    }
                });

                $elem.off('mousedown', '.TreeNode').on('mousedown', '.TreeNode', function () {
                    $(this).addClass('TreeNodeMouseDown');
                    var node = $(this);
                    var children = $(this).siblings('.children-container[data-pid=' + node.data('id') + ']');

                    var visible = children.is(':visible');
                    if (visible) {
                        children.hide();
                        node.find('.ExpandFlagContainer').addClass('FlagCollapsed').removeClass('FlagExpanded');
                    } else {
                        children.show();
                        node.find('.ExpandFlagContainer').removeClass('FlagCollapsed').addClass('FlagExpanded');
                    }
                })
                .off('mouseup', '.TreeNode').on('mouseup', '.TreeNode', function () {
                    $(this).removeClass('TreeNodeMouseDown');
                })
                .off('mouseover', '.TreeNode').on('mouseover', '.TreeNode', function () {
                    $(this).addClass('TreeNodeHovered');
                })
                .off('mouseout', '.TreeNode').on('mouseout', '.TreeNode', function () {
                    $(this).removeClass('TreeNodeHovered');
                }).off('click', '.TreeNode').on('click', '.TreeNode', function (e) {
                    if ($('.TreeNode[data-pid=' + $(this).data('id') + ']').length == 0) {
                        $('.TreeNodeSelected').removeClass('TreeNodeSelected');
                        $(this).addClass('TreeNodeSelected');
                    }
                    if (opt.onNodeClick && $.isFunction(opt.onNodeClick)) {
                        opt.onNodeClick(e, $(this));
                    }
                });
            }

            rendTrees(opt.data);
            bindEvent();
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