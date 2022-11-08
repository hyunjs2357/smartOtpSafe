<?php 
	
	//이게 핵심이다.
	function send_notification ($tokens, $data)
	{
		$url = 'https://fcm.googleapis.com/fcm/send';
		//어떤 형태의 data/notification payload를 사용할것인지에 따라 폰에서 알림의 방식이 달라 질 수 있다.
		$msg = array(
			'title'	=> $data["title"],
			'body' 	=> $data["body"]
          );

		//data payload로 보내서 앱이 백그라운드이든 포그라운드이든 무조건 알림이 떠도록 하자.
		$fields = array(
				'registration_ids'		=> $tokens,
				'data'	=> $msg
			);

		//구글키는 config.php에 저장되어 있다.
		$headers = array(
			'Authorization:key =' . GOOGLE_API_KEY,
			'Content-Type: application/json'
			);

	   $ch = curl_init();
       curl_setopt($ch, CURLOPT_URL, $url);
       curl_setopt($ch, CURLOPT_POST, true);
       curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
       curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
       curl_setopt ($ch, CURLOPT_SSL_VERIFYHOST, 0);  
       curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, false);
       curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode($fields));
       $result = curl_exec($ch);           
       if ($result === FALSE) {
           die('Curl failed: ' . curl_error($ch));
       }
       curl_close($ch);
       return $result;
	}
	

	//데이터베이스 저장된 토큰을 array변수에 모두 담는다.
	include_once 'config.php';
	$conn = mysqli_connect(DB_HOST, DB_USER, DB_PASSWORD, DB_NAME);
	
	$secretKey = $_POST['userSecretKey'];
	//echo $secretKey;
	$sql = "Select userToken From tableUser where userSecretKey='$secretKey'";

	$result = mysqli_query($conn,$sql);
	$tokens = array();
	if(mysqli_num_rows($result) > 0 ){
		//DB에서 가져온 토큰을 모두 $tokens에 담아 둔다.
		while ($row = mysqli_fetch_assoc($result)) {
			$tokens[] = $row["userToken"]; 
		}
	}
	mysqli_close($conn);

	//관리자페이지 폼에서 입력한 내용들을 가져와 정리한다.
	$mTitle = "Warning!";
	if($mTitle == '') $mtitle="Warning!";
    $mMessage = "Safe Move"; 
	//알림할 내용들을 취합해서 $data에 모두 담는다. 프로젝트 의도에 따라 다른게 더 있을 수 있다.
	$inputData = array("title" => $mTitle, "body" => $mMessage);

	//마지막에 알림을 보내는 함수를 실행하고 그 결과를 화면에 출력해 준다.
	$result = send_notification($tokens, $inputData);
	echo $result;

 ?>