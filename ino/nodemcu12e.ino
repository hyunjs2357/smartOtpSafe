#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>

/*  아두이노에서 OTP 생성할 때 쓰는 사용자 비밀키를 동일하게 변수 선언
 *  해당 값은 웹서버 접속시 DB에서 사용자 정보를 가져오기 위한 select문 뒤 where 조건절에 사용됨
 *  어떤 사용자의 금고가 움직이고 있는지 구별해야 모든 사용자가 아닌 해당 사용자에게만 알림메시지를 보낼 수 있음 */
const String userSecretKey = "safeotp001";

/* 와이파이 설정 */
const char* ssid = "BaeWha_WiFi"; // 와이파이 공유기 이름
const char* password = "baewha2018"; // 와이파이 공유기 비밀번호
const char* host = "safeotp1123.dothome.co.kr"; // 연결할 웹서버 주소

// 중요! 아두이노 핀 연결하는 것처럼 연결하는게 아니라 esp 8266 칩 근처 핀에 연결하는 것이었음(자세한 것은 와이파이 구매 관련 사이트 참고)
int tiltPin = 13; // GPIO13

void setup() {  
  delay(1000);
  Serial.begin(115200); // 와이파이 통신 속도가 115200이다.
  
  // 기울기 센서를 GPIO13번 핀에 연결하고 신호를 받는 입력핀으로 설정
  pinMode(tiltPin, INPUT);
 
  delay(10);

  /* 와이파이 연결 관련 부분 */
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); // 와이파이 공유기명이랑 비밀번호로 연결 시도

  while (WiFi.status() != WL_CONNECTED) { // 연결될때까지 .을 찍음
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  // 연결되면 시리얼 모니터에 나타남
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // 와이파이 공유기로 받은 ip 출력
}
 
void loop() { 
  int tilt = digitalRead(tiltPin); // 디지털 7번핀에 연결된 기울기 센서의 센서 값을 받아옴

  /* 누군가 금고를 옮기려고 할 때 관련 부분(
   * 알림 보내는 건 웹서버에서 처리함
   * 금고 이동 여부에 따라 이동했으면 사용자 비밀키를 웹서버로 전달, 이동안했으면 상태 유지 */
  if(tilt == 1){ 
    HTTPClient http;    // 사용하는 와이파이 장치를 클라이언트로 설정

    String postData; // 웹서버로 보낼 사용자 비밀키 값을 넣을 변수
    postData = "userSecretKey=" + userSecretKey ; // 사용자 비밀키 값을 변수에 대입

    http.begin("http://safeotp1123.dothome.co.kr/push_notification.php"); // 웹서버에서 알림 보내는 걸 처리하는 페이지 설정
    http.addHeader("Content-Type", "application/x-www-form-urlencoded"); 
  
    int httpCode = http.POST(postData);   // POST로 값 전달한 후 request code 받을 변수 (예시로 정상적으로 처리됐으면 200이 값으로 들어옴)
    String payload = http.getString();    
   
    Serial.println(httpCode);
    Serial.println(payload);    
   
    http.end();  //Close connection
    
    delay(5000);  //Post Data at every 5 seconds  
  }
  
}
