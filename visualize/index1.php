<!DOCTYPE html>
<html>
<head>
	<title>Doc Spider</title>
	<meta charset="utf-8">
	<meta name="viewport" content="width=device-width, initial-size=1.0">
	<link rel="stylesheet" type="text/css" href="assets/css/bootstrap.min.css">
	<link rel="stylesheet" type="text/css" href="assets/css/bootstrap.css">
	<link rel="stylesheet" type="text/css" href="assets/css/bootstrap-datetimepicker.min.css">
	<link rel="stylesheet" type="text/css" href="assets/css/daterangepicker-bs3.css">
	<style type="text/css">
		.table-hover>tbody>tr:hover{background-color:#f5f5f5;color: #111;}
		.nav-tabs{border: none;}
	</style>
	<script type="text/javascript" src="https://cdn.staticfile.org/jquery/3.1.1/jquery.min.js"></script>
	<script type="text/javascript" src="assets/js/bootstrap.min.js"></script>
	<script type="text/javascript" src="assets/js/moment.js"></script>
	<script type="text/javascript" src="assets/js/bootstrap-datetimepicker.js"></script>
	<script type="text/javascript" src="assets/js/bootstrap-datetimepicker.fr.js"></script>
	<script type="text/javascript" src="assets/js/daterangepicker.js"></script>
</head>
<body>
	<nav class="navbar navbar-inverse" role="navigation" style="border-radius: 0;">
		<div class="container-fluid">
			<div class="navbar-header">
				<a class="navbar-brand" href="#">Doc Spider</a>
			</div>
			<div>
				<ul class="nav navbar-nav">
					<li class="active"><a href="#">设置</a></li>
					<li><a href="#">控制</a></li>
					<li class="dropdown">
						<a href="#" class="dropdown-toggle" data-toggle="dropdown">
							帮助 <b class="caret"></b>
						</a>
						<ul class="dropdown-menu">
							<li><a href="https://www.baidu.com">1</a></li>
							<li><a href="#">2</a></li>
							<li><a href="#">3</a></li>
							<li class="divider"></li>
							<li><a href="#">4</a></li>
							<li class="divider"></li>
							<li><a href="#">5</a></li>
						</ul>
					</li>
				</ul>
			</div>
		</div>
	</nav>

	<div class="container">
		<div class="row">
			<div class="col-md-12" style="background-color: #eee; margin: 0 0 20px 0; border-radius: 10px;">
				<h1>A Simple Doc Spider Application</h1>
			</div>
		</div>
	</div>
	<div class="container">
		<div class="row">

			<ul id="myTab" class="nav nav-tabs">
				<li class="active col-md-4" style="padding: 0;">
					<button type="button" class="btn btn-primary btn-lg btn-block" style="border-radius: 10px 0 0 0;"><!-- <a href="#scan" data-toggle="tab" style="color: #fff; display: block;"> -->SETTINGS
					<!-- </a> --></button>
				</li>

				<li class="col-md-8" style="padding: 0;">
					<button type="button" class="btn btn-primary btn-lg btn-block" style="border-radius: 0 10px 0 0;"><!-- <a href="#record" data-toggle="tab" style="color: #fff; display: block;"> -->SCAN RESULTS<!-- </a> -->
					</button>
				</li>
			</ul>

			<div id="myTabContent" class="tab-content">

				<div class="tab-pane fade in active" id="scan">
					<div class="col-md-4" style="height: 70vh; min-height: 543px; margin-bottom: 25px; background-color: #eee; border-bottom-left-radius: 10px;">

						<br><br>
						<form role="form">
							<div class="form-group">
								<label for="name" class="input-lg">扫描目标设置：</label>
								<select class="form-control input-lg">
									<option>HOST</option>
									<option>网段</option>
								</select>
								<br>
								<input type="text" class="form-control input-lg" placeholder="请输入HOST或网段">
								<br>
								<button type="submit" class="btn btn-warning btn-block btn-lg">开始扫描</button>
							</div>
						</form>

					</div>
					<div class="col-md-8" style="height: 70vh; min-height: 543px; margin-bottom: 25px; background-color: #333; border-bottom-right-radius: 10px; overflow-y: scroll;">
						<table class="table table-condensed table-hover" style="color: #ccc; margin-top: 15px;">
							<thead>
								<tr>
									<th>网站</th>
									<th>URL</th>
								</tr>
								<?php
$conn = mysql_connect("localhost:8889","root","root");//连接mysql数据库并将返回结果赋值给变量conn
//接受参数名为id的值并赋值给变量x
//1.针对关键字过滤（正则表达式）
//$x = str_replace("union","x",$_GET['id']);
//2.判断传参类型（类型函数）
// $x = $_GET['id'];
// if(is_numeric($x)){
	mysql_select_db("sqlin",$conn);//选择连接请求为conn下的fanke数据库
	$sql="select * from user where id=5";
	$result = mysql_query($sql);//执行变量sql的语句并将返回结果赋值给变量result
//数组遍历结果选择显示
	while($row = mysql_fetch_array($result)){
		echo "用户ID：".$row['id']."<br >";
		echo "用户名：".$row['username']."<br >";
		echo "用户密码：".$row['password']."<br >";
	}

	mysql_close($conn);
	// echo '<hr>';
	// echo $sql;
?>
							</thead>
							<tbody>
							</tbody>
						</table>	
					</div>
				</div>
			</div>
		</div>
	</div>
</body>
</html>