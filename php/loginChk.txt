<?php 
	header('content-type: text/html; charset=utf-8'); 
	//데이터베이스에 접속해서 토큰저장
	include_once 'config.php'; 
	$conn = mysqli_connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME); 
	//DB 기본설정에 오류가 있을 경우
	if (mysqli_connect_errno()){
	  echo "Failed to connect to MySQL: " . mysqli_connect_error();
	}
	$id = $_POST['userid'];
	$pw = $_POST['userpw'];
	$token = $_POST['usertoken'];
	
	$queryLogin = "SELECT * FROM tableUser WHERE userID='$id' and userPW='$pw'";
	$queryToken = "UPDATE tableUser SET userToken = '$token' WHERE userID='$id'"; 
	
	$result = mysqli_query($conn, $queryLogin);
	//쓰기과정에 오류가 있을 경우
	if($result){
		mysqli_query($conn, $queryToken);
		$row = mysqli_fetch_array($result);
		echo $row["userBluetoothName"];
	}else{
		echo "fail";
	}
	mysqli_close($conn); 
?>