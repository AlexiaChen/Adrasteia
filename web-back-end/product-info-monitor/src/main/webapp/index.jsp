<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title>Show Product List</title>
<link href="controls/bootstrap/css/bootstrap-theme1.min.css"
	rel="stylesheet" />
<link href="controls/jQuery.Splitter1.1/splitter.css" rel="stylesheet" />
<link href="controls/jQuery.Tree2014/jquery.tree2014.css"
	rel="stylesheet" />
<link href="controls/jQuery.Table2014/Table2014.css" rel="stylesheet" />

<script type="text/javascript" src="./scripts/jquery-1.11.1.min.js"></script>
<script
	src="./controls/jQuery.Splitter1.1/jquery-ui-1.11.4-draggable.min.js"></script>
<script src="./controls/jQuery.Tree2014/jquery.tree2014.js"></script>
<script src="./controls/jQuery.Table2014/Table2014.js"></script>

<style>
body {
	width: 100%;
	height: 100%;
	margin: 0;
	padding: 0;
	overflow: hidden;
	margin: 0;
	padding: 0;
}

#header {
	position: absolute;
	top: 0px;
	left: 0px;
	right: 0px;
	height: 80px;
	background: -moz-linear-gradient(90deg, #2383d2 8%, #154a7d 88%);
	/* FF3.6+ */
	background: -webkit-gradient(linear, 90deg, color-stop(8%, #2383d2),
		color-stop(88%, #154a7d)); /* Chrome,Safari4+ */
	background: -webkit-linear-gradient(90deg, #2383d2 8%, #154a7d 88%);
	/* Chrome10+,Safari5.1+ */
	background: -o-linear-gradient(90deg, #2383d2 8%, #154a7d 88%);
	/* Opera 11.10+ */
	background: -ms-linear-gradient(90deg, #2383d2 8%, #154a7d 88%);
	/* IE10+ */
	filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#154a7d',
		endColorstr='#2383d2', GradientType='0'); /* for IE */
	background: linear-gradient(0deg, #2383d2 8%, #154a7d 88%); /* W3C */
	min-width: 870px;
}

.logo {
	position: absolute;
	top: 0px;
	bottom: 0px;
	left: 10px;
	right: 200px;
	background: url('./images/Logo.png') no-repeat;
}

.logo span {
	font-size: 36px;
	font-family: "微软雅黑";
	color: #ffffff;
	letter-spacing: 5px;
	height: 80px;
	line-height: 80px;
	text-shadow: 2px 2px 3px #000;
	margin-left: 100px;
}

div[data-layout=splitv] {
	top: 80px;
} 

div[data-layout=content] {
	overflow: auto;
	top: 0px;
}

/*标题栏强制不换行*/
.table-title-container {
	white-space: nowrap;
}
</style>
<script type="text/javascript">
	$(document).ready(function() {
		//调整一下页面样式
		$(".splitv-dragbar").css("left", "260px");
		$(".center").css("left", "265px");
		//加载左侧树
		getLeftTree();
	});
	
	//WebService地址
	var sendAddress = "/MQDispatch/mq/sendmonitor/list";
	var recvAddress = "/MQDispatch/mq/recvmonitor/list";
	var micapsAddress = "/MQDispatch/mq/localmicaps/list";
	var taskReprocessAddress = "/MQDispatch/mq/task/reprocess";
	var taskAddress = "/MQDispatch/mq/task/list";
	//右侧列表数据
	var dataTable = [];
	var url;
	//选中的节点
	var selectedNode;
	//右侧列表
	var grid;

	//得到左侧树
	function getLeftTree() {
		//新篇章 
		var nodes = [];
		nodes.push({
			"Expanded" : true,
			"Icon" : "listicon.png",
			"Id" : "1",
			"Name" : "MQ发送文件监控",
			"Pid" : "RoleMonitor",
			"Target" : ""
		}, {
			"Expanded" : true,
			"Icon" : "listicon.png",
			"Id" : "2",
			"Name" : "MQ接收文件监控(省)",
			"Pid" : "RoleMonitor",
			"Target" : ""
		}, {
			"Expanded" : true,
			"Icon" : "listicon.png",
			"Id" : "3",
			"Name" : "MQ接收文件监控(国)",
			"Pid" : "RoleMonitor",
			"Target" : ""
		}, {
			"Expanded" : true,
			"Icon" : "listicon.png",
			"Id" : "4",
			"Name" : "本地Micaps文件监控",
			"Pid" : "RoleMonitor",
			"Target" : ""
		}, {
			"Expanded" : true,
			"Icon" : "listicon.png",
			"Id" : "5",
			"Name" : "NWFD/LWFD任务处理（格点拼图）监控",
			"Pid" : "RoleMonitor",
			"Target" : ""
		});

		var treeData;
		treeData = {
			"TreeId" : "RoleMonitor",
			TreeTitle : "监控权限",
			Expanded : true,
			TreeIcon : "treeicon.png",
			"TreeNodes" : nodes
		};
		//创建左侧树
		var tree = $("#leftTree").tree2014({
			data : treeData,
			width : 250,
			onNodeClick : onTreeNodeSelected
		});
		//选中第一项
		if (treeData.TreeNodes.length > 0) {
			selectedNode = $(".TreeNode").eq(0);
			onTreeNodeSelected("", $(".TreeNode").eq(0));
			$(".TreeNode").eq(0).addClass("TreeNodeSelected");//第一项选中
		}
	}
	//点击左侧树节点触发事件
	function onTreeNodeSelected(e, el) {
		selectedNode = el;
		grid = null;
		//根据不同的节点，得到不同的数据源，展示不同的表格 
		url = "";
		dataTable = [];
		switch ($(el).data("id")) {
		case 1://MQ发送文件监控
			url = sendAddress;
			getSendData();
			break;
		case 2://MQ接收文件监控(省)
			url = recvAddress;
			getRecvData();
			break;
		case 3://MQ接收文件监控(国)
			url = recvAddress;
			getRecvDataWithCountry();
			break;
		case 4://本地Micaps文件监控
			url = micapsAddress;
			getMicapsData();
			break;
		case 5://NWFD/LWFD任务处理（格点拼图）监控 
			url = taskAddress;//todo:暂时先用Micaps的url
			getTaskStatus();
			break;
		default:
			break;
		}
	}
	//MQ发送文件监控
	function getSendData() {
		$.ajax({
			url : url,
			type : 'get',
			datatype : 'json',
			contentType : "application/json",
			success : function(result) {
				
				result = JSON.parse(result);
				
				if (typeof (result) != "undefined" && result) {
					$.each(result, function() {
						var name = this.name;//产品名称
						$.each(this.fileSet.m_files, function() {
							dataTable.push({
								"name" : name,
								"reportTime" : this.reportTime,
								"fileName" : this.fileName,
								"shouldTime" : this.shouldSendTime,
								"practicalTime" : this.sendTime,
								"result" : this.Result
							});
						});
					});
					getSendTable();
				}
			}
		});
	}
	//MQ接收文件监控(省)
	function getRecvData() {
		$.ajax({
			url : url,
			type : 'get',
			datatype : 'json',
			success : function(result) {
				
				result = JSON.parse(result);
				
				if (typeof (result) != "undefined" && result) {
					$.each(result, function() {
						var name = this.name;//产品名称 
						$.each(this.fileSet.m_files, function() {
							dataTable.push({
								"name" : name,
								"reportTime" : this.reportTime,
								"fileName" : this.fileName,
								"shouldTime" : this.shouldRecvTime,
								"practicalTime" : this.recvTime,
								"result" : this.result
							});
						});
					});
					getRecvTableWithoutCountry();
				}
			}
		});
	}
	//MQ接收文件监控(国)
	function getRecvDataWithCountry() {
		$.ajax({
			url : url,
			type : 'get',
			datatype : 'json',
			success : function(result) {
				
				result = JSON.parse(result);
				
				if (typeof (result) != "undefined" && result) {
					var provinceName;//省份名称
					$.each(result, function() {
						var name = this.name;//产品名称
						provinceName = this.ccccName;//省份名称 
						$.each(this.fileSet.m_files, function() {
							dataTable.push({
								"countryName" : provinceName,
								"name" : name,
								"reportTime" : this.reportTime,
								"fileName" : this.fileName,
								"shouldTime" : this.shouldRecvTime,
								"practicalTime" : this.recvTime,
								"result" : this.result
							});
						});
					});
					getRecvTableWithCountry();
				}
			}
		});
	}
	//本地Micaps文件监控
	function getMicapsData() {
		$.ajax({
			url : url,
			type : 'get',
			datatype : 'json',
			success : function(result) {
				result = JSON.parse(result);
				
				if (typeof (result) != "undefined" && result) {
					$.each(result, function() {
						var name = this.name;//产品名称 
						$.each(this.fileSet.m_files, function() {
							dataTable.push({
								"name" : name,
								"reportTime" : this.reportTime,
								"fileName" : this.fileName,
								"shouldTime" : this.shouldRecvTime,
								"practicalTime" : this.recvTime,
								"result" : this.Result
							});
						});
					});
					//根据选中节点的Id来决定
					if ($(selectedNode).data("id") == 4) {//本地Micaps文件监控
						getMicapsTable();
					} else if ($(selectedNode).data("id") == 5) {
						getTaskTable();
					}
				}
			}
		});
	}
	
	//NWFD/LWFD任务处理（格点拼图）监控
	function getTaskStatus(){
		$.ajax({
			url : url,
			type : 'get',
			datatype : 'json',
			success : function(result){
				
				result = JSON.parse(result);
				
				if (typeof (result) != "undefined" && result){
					
					
					$.each(result, function() {
						
						var name = this.name;//产品名称 
						var productkey = this.productKey;
						var reportTimes = this.reporttimes;
						
						if(typeof (this.fileSet.m_files) != "undefined" 
								&& this.fileSet.m_files){
							
							
								
							$.each(this.fileSet.m_files,function(){
								dataTable.push({
									"key"  : productkey,
									"name" :  name,
									"reportTime" : this.reportTime,
									"fileName" :  this.fileName,
									"shouldTime" :  this.shouldTime,
									"practicalTime" :  this.practicalTime,
									"result" :  this.Result
								});
							});
							
							
							
							     
							
						}
						
						
						
					});
					
					if ($(selectedNode).data("id") == 5) {
						getTaskTable();
					}
				}
			}
		});
	}
	
	function reprocess(product_key,report_time){
		$.ajax({
			url:taskReprocessAddress,
			type : 'get',
			data:{
				"productKey":product_key,
				"reportTime":report_time
			},
			success:function (result){}
		});
	}

	//生成发送文件表格
	function getSendTable() {
		grid = $("#divSendTable")
				.table2014(
						{
							data : {
								rows : dataTable
							},
							tableTitle : "MQ发送文件监控",
							tableHeader : true,
							hoverColor : true,
							dataType : 'jsonp',
							dataRowsPath : 'rows',
							columns : [
									{
										header : '产品名称',
										colName : 'name',
										width : '150'
									},
									{
										header : '时次',
										colName : 'reportTime',
										width : '50',
										align : 'left'
									},
									{
										header : '文件名称',
										colName : 'fileName',
										//width : '500',
										align : 'left',
										render : function(row) {
											return '<div title="'+row.fileName+'">'
													+ row.fileName + '</div>';
										}
									},
									{
										header : '应发时间',
										colName : 'shouldTime',
										width : '100',
										align : 'left'
									},
									{
										header : '实际发送时间',
										colName : 'practicalTime',
										width : '120',
										align : 'left'
									},
									{
										header : '发送结果',
										colName : 'result',
										width : '100',
										render : function(row) {
											if (row.Result == true) {
												return '<div class="Success">发送成功</div>';
											} else if (row.result == false) {
												return '<div class="Error">发送失败</div>';
											} else {
												return '<div class="noFile">文件未产成</div>';
											}
										}
									} ],
						});
		if (grid) {
			changeRowColor();
		}
	}

	//生成表格，MQ接收文件监控(省) 
	function getRecvTableWithoutCountry() {
		grid = $("#divSendTable").table2014(
				{
					data : {
						rows : dataTable
					},
					tableTitle : "MQ接收文件监控(省)",
					tableHeader : true,
					hoverColor : true,
					dataType : 'jsonp',
					dataRowsPath : 'rows',
					columns : [
							{
								header : '产品名称',
								colName : 'name',
								width : '200'
							},
							{
								header : '时次',
								colName : 'reportTime',
								width : '50',
								align : 'left'
							},
							{
								header : '文件名称',
								colName : 'fileName',
								//width : '500',
								align : 'left',
								render : function(row) {
									return '<div title="'+row.fileName+'">'
											+ row.fileName + '</div>';
								}
							}, {
								header : '应到时间',
								colName : 'shouldTime',
								width : '100',
								align : 'left'
							}, {
								header : '实到时间',
								colName : 'practicalTime',
								width : '100',
								align : 'left'
							}, {
								header : '发送结果',
								colName : 'result',
								width : '100',
								render : function(row) {
									return RecvResult(row);
								}
							} ],
				});
		if (grid) {
			changeRowColor();
		}
	}

	//生成表格,MQ接收文件监控(国) 
	function getRecvTableWithCountry() {
		grid = $("#divSendTable").table2014(
				{
					data : {
						rows : dataTable
					},
					tableTitle : "MQ接收文件监控(国)",
					tableHeader : true,
					hoverColor : true,
					dataType : 'jsonp',
					dataRowsPath : 'rows',
					columns : [
							{
								header : '省份',
								colName : 'countryName',
								width : '150'
							},
							{
								header : '产品名称',
								colName : 'name',
								width : '200'
							},
							{
								header : '时次',
								colName : 'reportTime',
								width : '50',
								align : 'left'
							},
							{
								header : '文件名称',
								colName : 'fileName',
								//width : '500',
								align : 'left',
								render : function(row) {
									return '<div title="'+row.fileName+'">'
											+ row.fileName + '</div>';
								}
							}, {
								header : '应到时间',
								colName : 'shouldTime',
								width : '100',
								align : 'left'
							}, {
								header : '实到时间',
								colName : 'practicalTime',
								width : '100',
								align : 'left'
							}, {
								header : '发送结果',
								colName : 'result',
								width : '100',
								render : function(row) {
									return RecvResult(row);
								}
							} ],
				});
		if (grid) {
			changeRowColor();
		}
	}

	//生成表格，本地Micaps文件监控 
	function getMicapsTable() {
		grid = $("#divSendTable")
				.table2014(
						{
							data : {
								rows : dataTable
							},
							tableTitle : "本地Micaps文件监控",
							tableHeader : true,
							hoverColor : true,
							dataType : 'jsonp',
							dataRowsPath : 'rows',
							columns : [
									{
										header : '产品名称',
										colName : 'name',
										width : '150'
									},
									{
										header : '时次',
										colName : 'reportTime',
										width : '50',
										align : 'left'
									},
									{
										header : '文件名称',
										colName : 'fileName',
										//width : '500',
										align : 'left',
										render : function(row) {
											return '<div title="'+row.fileName+'">'
													+ row.fileName + '</div>';
										}
									},
									{
										header : '应到时间',
										colName : 'shouldTime',
										width : '100',
										align : 'left'
									},
									{
										header : '实到时间',
										colName : 'practicalTime',
										width : '100',
										align : 'left'
									},
									{
										header : '发送结果',
										colName : 'result',
										width : '100',
										render : function(row) {
											if (row.result) {
												return '<div class="Success">文件已到</div>';
											} 
											else{
												return '<div class="Error">文件未到</div>';
											}
										}
									} ],
						});
		if (grid) {
			changeRowColor();
		}
	}
	//生成表格，NWFD/LWFD任务处理（格点拼图）监控
	function getTaskTable() {
		grid = $("#divSendTable")
				.table2014(
						{
							data : {
								rows : dataTable
							},
							tableTitle : "NWFD/LWFD任务处理（格点拼图）监控",
							tableHeader : true,
							hoverColor : true,
							dataType : 'jsonp',
							dataRowsPath : 'rows',
							columns : [
									{
										header : '要素',
										colName : 'key',
										width : '0'
									
									},  
									{
										header : '产品名称',
										colName : 'name',
										width : '150'
									
									},
									{
										header : '时次',
										colName : 'reportTime',
										width : '50',
										align : 'left'
									},
									{
										header : '文件名称',
										colName : 'fileName',
										//width : '500',
										align : 'left',
										render : function(row) {
											return '<div title="'+row.fileName+'">'
													+ row.fileName + '</div>';
										}
									},
									{
										header : '计划生成时间',
										colName : 'shouldTime',
										width : '120',
										align : 'left'
									},
									{
										header : '实际生成时间',
										colName : 'practicalTime',
										width : '120',
										align : 'left'
									},
									{
										header : '发送结果',
										colName : 'result',
										width : '100',
										render : function(row) {
											if (row.result == true) {
												return '<div class="Success">生成成功</div>';
											} else {
												return '<div class="Error">生成失败</div>';
											}
										}
									},
									{
										header : '操作',
										width : '200',
										render : function(row) {
									
											return '<button class="btn btn-default" onclick="reprocess(\'{0}\',\'{1}\')" '.format(row.key,row.reportTime) + 
											'style="height: 25px;line-height: 14px;">重新生成</button>&nbsp;&nbsp;&nbsp;';
										}
									} ],
						});
		if (grid) {
			changeRowColor();
		}
	}

	//MQ接收文件监控 结果
	function RecvResult(row) {
		if (row.result == true) {
			return '<div class="Success">文件已到</div>';
		} else if (row.result == false) {
			return '<div class="Error">文件迟到</div>';
		} else {
			return '<div class="noFile">文件未到</div>';
		}
	}

	//根据发送结果改变行颜色
	function changeRowColor() {
		$(".noFile").parent().parent().css("color", "blue");
		//$(".Success").parent().parent().css("color", "green");
		$(".Error").parent().parent().css("color", "red");
	} 
</script>
</head>
<body>
	<div id="header">
		<div class="logo">
			<span>LWFD系统监控</span>
		</div>
	</div>

	<div data-layout="splitv">
		<div data-layout="left" style="padding: 5px;">
			<div id="leftTree"></div>
		</div>

		<div class="splitv-dragbar">
			<div class="dragbar-collapse"></div>
		</div>

		<div data-layout="center" class="center">
			<div data-layout="content">
				<div id="divSendTable"></div>
			</div>
		</div>
	</div>
	<script src="./controls/jQuery.Splitter1.1/splitter.js"></script>
</body>
</html>