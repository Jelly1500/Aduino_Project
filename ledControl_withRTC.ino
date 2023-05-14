// CONNECTIONS:
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 2
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND

// RTC모듈 라이브러리
#include <RtcDS1302.h>

// 기본 세팅
ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

// led 핀 번호
const int ledPin = 3;

// mode : 4*4키패드에서 led를 켜는 시간을 수정할 것인지, 끄는 시간을 수정할 것인지 알려준다.
char mode;
// count : 4*4 키패드에서 숫자를 입력받는 횟수
int count = 0;
// time : 키패드에서 입력받은 숫자를 저장
int time[4] = {0,0,0,0};
// isComplete_Input : 숫자 입력이 끝났는지 확인
bool isComplete_Input = false;

// led 현재 상태를 나타냄
bool isTurnOn_LED = false;
bool isTurnOff_LED = false;

// ledTime_ON : 켜는 시간을 저장
// ledTime_OFF : 끄는 시간을 저장
int ledTime_ON[4] = {0,6,0,0};
int ledTime_OFF[4] = {1,8,0,0};

// 4*4 키패드 세팅
const int numRows = 4;
const int numCols = 4;

char keys[numRows][numCols] = {
  {'*','0','#','D'},
  {'7','8','9','C'},
  {'4','5','6','B'},
  {'1','2','3','A'}
};

int rowPins[] = {6,7,8,9};
int colPins[] = {10,11,12,13};

void setup() {
  // 아두이노 LED 설정
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // 4 * 4 키패드 설정
  // 열을 입력 풀업 모드로 설정한다.
  for(int i = 0; i < numRows;i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  // 행을 출력 모드로 설정, 초기값을 HIGH로 설정한다.
  for(int i = 0; i < numCols;i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], HIGH);
  }
  // 시리얼 모니터 설정
  Serial.begin(57600);

  // RTC 모듈 설정(68~110)
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  
  Serial.println();

  if (!Rtc.IsDateTimeValid()) 
  {
    // Common Causes:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (Rtc.GetIsWriteProtected())
  {
    Serial.println("RTC was write protected, enabling writing now");
    Rtc.SetIsWriteProtected(false);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled) 
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled) 
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled) 
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }  
}

void loop() {
  // 현재 시간을 최신화
  RtcDateTime now = Rtc.GetDateTime();
  // 만약 now와 ledTime_ON의 값과 같다면(시, 분)
  if(now.Hour() == (ledTime_ON[0] * 10 + ledTime_ON[1]) && now.Minute() == (ledTime_ON[2] * 10 + ledTime_ON[3]) && !isTurnOn_LED) {
    isTurnOff_LED = false;
    isTurnOn_LED = true;
    ledOn(ledPin);
    Serial.println("LED ON");
  }
  // 만약 now와 ledTime_OFF의 값과 같다면(시, 분)
  if(now.Hour() == (ledTime_OFF[0] * 10 + ledTime_OFF[1]) && now.Minute() == (ledTime_OFF[2] * 10 + ledTime_OFF[3]) && !isTurnOff_LED) {
    isTurnOn_LED = false;
    isTurnOff_LED = true;
    ledOff(ledPin);
    Serial.println("LED OFF");
  }
  // ledOff 함수를 실행
  
  // key 변수에 키패드 입력값을 읽어서 저장한다.
  char key = keypadRead();
  // key 변수가 0일때는 입력이 없는 것이다.
  // 그 외의 값에서는 입력이 발생한 것이다.
  if(key != 0) {
    modify(key);
  }
  
}

#define countof(a) (sizeof(a) / sizeof(a[0]))

// 키패드 값을 읽어오기
char keypadRead() {
  char key = 0;
  for(int i = 0; i < numCols; i++) {
    digitalWrite(colPins[i], LOW);
    for(int j = 0; j < numRows; j++) {
      if(digitalRead(rowPins[j]) == LOW) {
        delay(100);
        while(digitalRead(rowPins[j]) == LOW);
        key = keys[j][i];
      }
    }
    digitalWrite(colPins[i], HIGH);
  }
  return key;
}

// led 켜기
void ledOn(int pinNum) {
  digitalWrite(pinNum, HIGH);
}
// led 끄기
void ledOff(int pinNum) {
  digitalWrite(pinNum, LOW);
}

// 입력값을 처리하는 함수
void modify(char key) {
  Serial.print("내가 누른 버튼 : ");
  Serial.print(key);
  Serial.println(" key");
  // key 값이 숫자이고, mode가 A(LED를 키기 시작하는 시간을 수정) 또는 B(LED를 끄기 시작하는 시간)라면
  if(isDigit(key) && (mode == 'A' || mode == 'B')) {
    //시간과 분을 입력받아야 한다. 총 4개의 정수를 입력받도록 설정한다.
    time[count] = int(key - 48);
    count++;
    //입력을 모두 마쳤다면
    if(count == 4) {
      // time에 값을 저장
      Serial.print("수정될 시간 : ");
      for(int i = 0; i<count; i++) {
        Serial.print(time[i]);
        if(i == 1) {Serial.print(":");}
      }
      Serial.println();
      Serial.println("만약 수정할 시간을 잘못 입력했다면 #(초기화) 버튼을 눌러주세요");
      Serial.println("이대로 수정하고 싶다면 *(수정/확인) 버튼을 눌러주세요");
      count = 0;
      isComplete_Input = true;
    }
  }
  // key 값이 문자라면
  else {
    //key 값이 A 또는 B라면
    if((key == 'A' || key == 'B') && count == 0) {
      // 입력을 받기 위한 세팅을 한다.
      Serial.print("수정 모드 : ");
      if(key == 'A') {Serial.println("LED를 켤 시간을 수정하겠습니다.");}
      else {Serial.println("LED를 끌 시간을 수정하겠습니다.");}
      mode = key;
      isComplete_Input = false;
    }
    //mode가 설정되고, 수정값을 다 받았다면, 
    else if(key == '*' && isComplete_Input) {
      //mode가 A일 때(켜는 시간을 수정)
      if(mode == 'A') {
        Serial.print("LED를 켤 시간이 수정되었어요! -> ");
        // time에 있는 값을 ledTime_ON으로 옮김
        for(int i=0;i<4;i++){
          ledTime_ON[i] = time[i];
          Serial.print(ledTime_ON[i]);
          if(i == 1) {Serial.print(":");}
        }
        Serial.println();
      }
      //mode가 B일 때(끄는 시간을 수정)
      else if(mode == 'B') {
        Serial.print("LED를 끌 시간이 수정되었어요! -> ");
        // time에 있는 값을 ledTime_OFF으로 옮김
        for(int i=0;i<4;i++){
          ledTime_OFF[i] = time[i];
          Serial.print(ledTime_OFF[i]);
          if(i == 1) {Serial.print(":");}
        }
        Serial.println();
      }
    }
    //key 값이 # 이라면(수정값 및 mode 초기화)
    else if(key == '#') {
      //수정값을 잘못 입력해서 다시 초기화
      Serial.println("수정이 취소되었습니다. 처음부터 진행해 주세요");
      mode = ' ';
      count = 0;
      isComplete_Input = false;
      for(int i=0;i<4;i++){
        time[i]=0;
      }
    }
    // LED 켜는 시간을 출력
    else if(key == 'C') {
      Serial.print("현재 설정된 LED 켜는 시간 : ");
      for(int i = 0; i < 4; i++) {
        Serial.print(ledTime_ON[i]);
        if(i == 1) { Serial.print(" : "); }
      }
      Serial.println();
    }
    // LED 끄는 시간을 출력
    else if(key == 'D') {
      Serial.print("현재 설정된 LED 끄는 시간 : ");
      for(int i = 0; i < 4; i++) {
        Serial.print(ledTime_OFF[i]);
        if(i == 1) { Serial.print(" : "); }
      }
      Serial.println();
    }
  }
}
