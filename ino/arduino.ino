#include <SoftwareSerial.h> // 블루투스 관련 헤더 파일
#include <Wire.h> // lcd 관련 헤더 파일
#include <LiquidCrystal_I2C.h> // lcd 관련 헤더 파일
#include <Keypad.h> // 키패드 관련 헤더 파일
#include <Servo.h> // 서보모터 관련 헤더 파일

/* OTP 관련 헤더파일 */
#include "sha1.h" 
#include "TOTP.h"
#include "swRTC.h"

/* OTP 관련 변수 선언 부분*/
// OTP 생성을 위한 사용자 비밀키 (여기서는 safeotp001을 비밀키로 사용함) -> 아스키코드표에서 16진수로 변경하면 됨
uint8_t hmacKey[] = {0x73, 0x61, 0x66, 0x65, 0x6f, 0x74, 0x70, 0x30, 0x30, 0x31};

TOTP totp = TOTP(hmacKey, 10);
swRTC rtc;
char code[7];

Servo locker; // 서버 모터 변수
LiquidCrystal_I2C lcd(0x27,16,2); // lcd 설정

/* 블루투스 설정 (연결된 TX, RX와 반대로 작성해야 함) */
#define BT_RXD 13
#define BT_TXD 12
SoftwareSerial bluetooth(BT_RXD, BT_TXD);

/* 금고 동작 관련 변수 선언 부분 */
String secretCode; // 생성된 OTP 코드를 금고 비밀번호로 저장하기 위한 변수
char input[6]; // 사용자로부터 입력받은 전체 값 저장 배열 변수
char key; // 사용자로부터 입력받은 개별 값 저장 변수
int wrong = 0; // 비밀번호와 입력값 비교해서 틀리면 증가되는 변수
int i = 0; // input 배열 index 지정 변수 (input_key함수 안 for문에서 사용)
int j = 0; // secretCode 배열, index 배열 index 지정 변수 (password함수 안 for문에서 사용)

int pass = 0; // 통과 여부 확인을 위한 변수

/* 키패드 설정 */
const byte rows = 4; // 행 
const byte cols = 3; // 열
char keys[rows][cols] =
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

byte rowPins[rows] = {2, 3, 4, 5};
byte colPins[cols] = {6, 7, 8};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

/* 비밀번호랑 입력값에 관여하는 함수들 */
void input_key(); // 입력받은 값 저장 함수
void password(); // 비밀번호랑 입력값 비교하는 함수
void password_check(); // 비밀번호랑 입력값 비교 시 틀렸는지 여부에 따라 lcd 다르게 보여줄 함수

void setup(){
  Serial.begin(9600);
  bluetooth.begin(9600);

  /* lcd 설정 부분 */
  lcd.init(); // lcd 초기화
  lcd.backlight(); // 백라이트 켜기
  lcd.begin(16, 2);

  /* 금고 설정 부분 */
  locker.attach(10); 
  locker.write(90);
  pinMode(10, INPUT);

  /* OTP 생성을 위한 시간 값 설정 부분 */
  rtc.stopRTC();
  rtc.setDate(15, 11, 2019); // 날짜 설정
  rtc.setTime(12, 10, 00); // 시간 설정
  rtc.startRTC();
}
 
void loop(){
  /* 앱과 아두이노가 블루투스로 연결되어 있을 경우 관련 부분
   * 블루투스가 연결되어 있으면 금고가 동작되도록 함 */
  if(bluetooth.available()){
    byte data = bluetooth.read(); 
    Serial.write(data);
    if(data == 'o'){ // 앱과 블루투스로 연결 잘됐다는 데이터가 왔을 경우
      long GMT = rtc.getTimestamp(); 
      char* newCode = totp.getCode(GMT);
      if(strcmp(code, newCode) != 0) { // 전에 생성된 OTP 코드랑 새로 생성된 코드랑 같지 않을 경우
        strcpy(code, newCode); 
        Serial.println(code);
        bluetooth.write(code); // 생성된 OTP 코드를 앱에 전달

        /* 금고가 잠겨있는 상태 */
        while (pass == 0) {
          input_key();            // 비밀번호 입력받는 함수 호출
          password();             // 생성된 OTP랑 동일한지 확인하는 함수 호출
          password_check();       // OTP랑 입력값이랑 동일여부에 따라 lcd 표시 처리하는 함수 호출
        }

        /* OTP랑 입력값이랑 동일할 경우 */
        if(pass == 1){
          lcd.clear();
          lcd.setCursor(0, 0);
      
          pinMode(10, OUTPUT);
          locker.write(180);      // 금고 열기
      
          lcd.print("perfect complete");
          delay(2000);
          
          pinMode(10, INPUT);
          key = keypad.waitForKey();

          /* 금고를 연 후에 다시 잠그려고 할 경우(키패드에서 * 입력했을 때) */
          if (key == '*') {
            pass = 0;                
            lcd.clear();
            lcd.setCursor(0, 0);
            
            pinMode(10, OUTPUT);
            locker.write(90);     // 금고 잠그기
            
            lcd.print("    Lock ON");
            delay(2000);
            
            pinMode(10, INPUT);
          }
        }
      }  
    }
  }
}

/* 키패드에서 입력한 값 저장하는 함수 */
void input_key(){ 
  lcd.clear(); 
  lcd.setCursor(0, 0);
  lcd.print("    Password");
  for (i = 0; i < 6; i++)  {
    key = keypad.waitForKey();
    input[i] = key; // 입력한 키 값 저장
    lcd.setCursor(i + 5, 1);
    lcd.print(key);
  }
}

/* 입력한 값이랑 OTP값 비교하는 함수 */
void password(){ // 
  wrong = 0;
  for (j = 0; j < 6; j++) { // 입력한 값이랑 비밀번호를 한 문자씩 비교하는 for문
    if (code[j] == input[j]) {
    } else if (code[j] != input[j]) { // 비밀번호랑 입력값 다르면
      wrong += 1; 
    }
  }
}

/* OTP랑 입력값 비교한 결과에 따라 lcd 다르게 출력해주는 함수 */
void password_check(){ // 
  if (wrong == 0)  { // 비밀번호 맞았을 때
    Serial.println("correct");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("      PASS");
    delay(2000);
    pass = 1;
    lcd.clear();
  }  else if (wrong != 0)  { // 비밀번호 틀렸을 때
    Serial.println("wrong");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("     wrong");
    delay(2000);
    pass = 0;
    lcd.clear();
    wrong = 0;
  }
}
