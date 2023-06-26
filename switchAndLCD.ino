#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RtcDS1302.h>

// 연결(RTC 모듈)
// DS1302 CLK/SCLK --> 5
// DS1302 DAT/IO --> 4
// DS1302 RST/CE --> 3
// DS1302 VCC --> 3.3v - 5v
// DS1302 GND --> GND

// 연결(조이스틱)
// GND --> GND
// +5V --> 5v
// VRX --> A0
// VRY --> A1
// SW --> 2

// 연결(LCD)
// GND --> GND
// VCC --> 5V
// SDA --> A4
// SDL --> A5

// 연결(RED LED)
// (-) --> 저항
// 저항 --> GND
// (+) --> 13

// 내가 쓰는 lcd 주소값 : 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

// RTC 모듈 초기 설정
ThreeWire myWire(4,5,3); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);

const int ledPin = 13;
//0번 아날로그핀을 X축 입력으로 설정
const int xAxisPin = 0;
const int yAxisPin = 1;
const int zAxisPin = 2;

// 메인 메뉴
const String mainMenu[2] = {
  "CHANGE TIME",
  "SHOW TIME"
};

// 시간 선택 메뉴
const String choiceTimeMenu[2] = {
  "START TIME",
  "END TIME"
};

// 설정된 시작 시간과 종료 시간을 보여주기
const String showTimes[2] = {
  "START:",
  "END:"
};

const String editTimes[2] = {
  "HOUR:",
  "MIN:"
};

// selectTime == 0 : 시작 시간을 가리킴
// selectTime == 1 : 종료 시간을 가리킴
int selectTime = -1;
// selectMode == 0 : 시간 바꾸기 모드를 가리킴
// selectMode == 1 : 시간 보여주기 모드를 가리킴
int selectMode = -1;

// currentDisplay == 0 : 현재 메인 메뉴를 보여주고 있음
// currentDisplay == 1 : 현재 시간 선택 메뉴를 보여주고 있음
// currentDisplay == 2 : 현재 시간을 수정하는 설정창을 보여주고 있음 
// currentDisplay == 3 : 현재 설정된 시간을 보여주고 있음
int currentDisplay = 0;

// LCD 커서 위치를 나타냄
int cursorX = 1;
int cursorY = 0;
// 시간 수정 시에 각 요소에 접근하는 커서의 x축 위치
const int leftX = 1;
const int rightX = 9;


// LED 켜는 시간
int startTime[2] = {0, 0};
// LED 끄는 시간
int endTime[2] = {0, 0};

int moveCursorY(int yAxis) {
  int y;
  if (yAxis == 0) { y = 0; }
  else { y = 1; }
  return y;
}

int moveCursorX(int xAxis, int moveX) {
  int x;
  if (xAxis == 0) { x = leftX; }
  else { x = rightX; }
  return x;
}

// LCD에서 메뉴 창에 보이는 커서의 위치를 변경
void changeMenuCursor(LiquidCrystal_I2C lcd, int yAxis) {
  lcd.setCursor(0, cursorY);
  lcd.print(" ");
  delay(400);
  cursorY = moveCursorY(yAxis);
  lcd.setCursor(0, cursorY);
  lcd.print(">");
}
//LCD에서 시간을 수정하는 창에 보이는 커서의 위치를 변경
void changeEditCursor(LiquidCrystal_I2C lcd, int xAxis, int moveX) {
  lcd.setCursor(cursorX, 1);
  lcd.print(" ");
  delay(400);
  cursorX = moveCursorX(xAxis, moveX);
  lcd.setCursor(cursorX, 1);
  lcd.print(">");
}

// 메인 메뉴를 LCD에 출력하는 함수
void drawMainMenu(LiquidCrystal_I2C lcd) {
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print(mainMenu[0]);
  lcd.setCursor(1, 1);
  lcd.print(mainMenu[1]);
  changeMenuCursor(lcd, 0);
}

// 시간 선택 메뉴를 LCD에 출력하는 함수
void drawChoiceTimeMenu(LiquidCrystal_I2C lcd) {
  lcd.clear();
  lcd.setCursor(1,0);
  lcd.print(choiceTimeMenu[0]);
  lcd.setCursor(1, 1);
  lcd.print(choiceTimeMenu[1]);
  changeMenuCursor(lcd, 0);
}

// 시간을 수정하는 창을 LCD에 출력
void drawEditTimePage(LiquidCrystal_I2C lcd, int* Times, int cursorX) {
  // 시간을 출력
  lcd.clear();
  lcd.setCursor(leftX+1, 0);
  lcd.print(editTimes[0]);
  lcd.setCursor(leftX+2, 1);
  // 현재 설정된 시간을 출력
  if (Times[0] < 10) { lcd.print("0"); }
  String s = String(Times[0]);
  lcd.print(s);
  lcd.print("  :  ");

  // 분을 출력
  lcd.setCursor(rightX+1, 0);
  lcd.print(editTimes[1]);
  lcd.setCursor(rightX+2, 1);
  if (Times[1] < 10) { lcd.print("0"); }
  s = String(Times[1]);
  lcd.print(s);

  // 현재 커서의 위치를 출력
  lcd.setCursor(cursorX, 1);
  lcd.print(">");
}

// 설정된 시간을 보여주는 창을 LCD에 출력
void drawSettingTime(LiquidCrystal_I2C lcd) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(showTimes[0]);
  if (startTime[0] < 10) {
    lcd.print("0");
  }
  String s = String(startTime[0]);
  lcd.print(s);
  lcd.print(":");
  if (startTime[1] < 10) {
    lcd.print("0");
  }
  s = String(startTime[1]);
  lcd.print(s);
  lcd.setCursor(0, 1);
  lcd.print(showTimes[1]);
  if (endTime[0] < 10) {
    lcd.print("0");
  }
  s = String(endTime[0]);
  lcd.print(s);
  lcd.print(":");
  if (endTime[1] < 10) {
    lcd.print("0");
  }
  s = String(endTime[1]);
  lcd.print(s);
}

// 시간에 대한 숫자를 증가시키거나 감소시킬 때 사용하는 함수
// 입력받는 값 : yValue, cursorX, 시간값을 담은 배열(startTime, endTime 둘 중 하나)
void editTime(int moveX, int yValue, int cursorX, int* timeArr) {
  // 만약 cursorX == 0이라면 (시간을 변경하고 싶다면)
  if (cursorX == leftX) {
    // yValue == 0이라면 (숫자를 증가시키고 싶다면, 레버를 위로 올렸다면)
    if (yValue == 0) {
      timeArr[0]++;
      // 시간의 범위는 0~23으로 조정
      if (timeArr[0] >= 24) {
        timeArr[0] = 0;
      }
    }
    // yValue == 1023 이라면 (숫자를 감소시키고 싶다면)
    else if (yValue == 1023) {
      timeArr[0]--;
      // 시간의 범위는 0~23으로 조정
      if (timeArr[0] < 0) {
        timeArr[0] = 23;
      }
    }
  }
  // 만약 x축의 위치가 moveX라면 (분을 변경하기를 희망한다면)
  else if (cursorX == rightX) {
    // yValue == 0이라면 (숫자를 증가시키고 싶다면, 레버를 위로 올렸다면)
    if (yValue == 0) {
      timeArr[1]+=5;
      // 분의 범위는 0~59로 조정
      if (timeArr[1] >= 60) {
        timeArr[1] = 0;
      }
    }
    // yValue == 1023이라면 (숫자를 감소시키고 싶다면, 레버를 아래로 내렸다면)
    else if (yValue == 1023) {
      timeArr[1]-=5;
      // 분의 범위는 0~59로 조정
      if (timeArr[1] < 0) {
        timeArr[1] = 55;
      }
    }
  }
  delay(400);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);
  Serial.begin(57600);
  Rtc.Begin();
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
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
    Rtc.SetDateTime(compiled);
  }
  else if (now == compiled) 
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }
  //z축 입력은 디지털 입력으로 설정한다.
  pinMode(zAxisPin, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  drawMainMenu(lcd);
}

void loop() {
  int xValue = analogRead(xAxisPin);
  int yValue = analogRead(yAxisPin);
  int zValue = digitalRead(zAxisPin);
  // RTC에서 현재 시간을 받아온다.
  RtcDateTime now = Rtc.GetDateTime();
  if (!now.IsValid())
  {
    // Common Causes:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }
  
  // 현재 시간과 led를 켜는 시간이 같아진다면
  if (now.Hour() == startTime[0] && now.Minute() == startTime[1]) {
    digitalWrite(ledPin, HIGH);
  }
  // 현재 시간과 led를 끄는 시간이 같아진다면
  else if(now.Hour() == endTime[0] && now.Minute() == endTime[1]) {
    digitalWrite(ledPin, LOW);
  }

  // y축을 움직일 때
  if (yValue == 0 || yValue == 1023) {
    // currentDisplay == 2 : 현재 시간을 수정하는 설정창을 보여주고 있다면 
    if (currentDisplay == 2) {
      //내가 가리킨 숫자를 바꾼다.
      // 만약 selectTime == 0이면 (startTime을 수정하기로 했다면)
      if (selectTime == 0) {
        // startTime을 적절히 수정한다.
        editTime(leftX, yValue, cursorX, startTime);
        drawEditTimePage(lcd, startTime, cursorX);
      }
      // 만약 selectTime == 1이면 (endTime을 수정하기로 했다면)
      else if (selectTime == 1) {
        // endTime을 적절히 수정한다.
        editTime(rightX, yValue, cursorX, endTime);
        drawEditTimePage(lcd, endTime, cursorX);
      }
      delay(400);
    }
    // 그 이외의 경우에는 단지 메뉴를 선택하는 기능만 필요하므로,
    else if (currentDisplay != 3) {
      // 커서를 위아래로 움직인다.
      changeMenuCursor(lcd, yValue);
    }
    // 설정 시간을 보여주고 있을 때에는 커서를 움직이지 않는다.
  }
  // x축을 움직일 때
  else if ((xValue == 0 || xValue == 1023)) {
    delay(400);
    // 현재 보여주고 있는 창이 설정 시간의 시와 분을 수정하는 창이라면
    if (currentDisplay == 2) {
      // 시간을 바꿀 것인지 분을 바꿀 것인지를 선택할 수 있다.
      // 만약 xValue == 0 이라면 시간을 바꾸는 것을 선택한 것이고,
      if (xValue == 0) {
        changeEditCursor(lcd, xValue, leftX);
      }
      // 만약 xValue == 1023이라면 분을 바꾸는 것을 선택한 것이다.
      else if (xValue == 1023) {
        changeEditCursor(lcd, xValue, rightX);
      }
    }
    // 그 이외의 창이라면
    else {
      // 현재 페이지가 메인 메뉴가 아니거나, 시간을 수정하는 창이 아니라면 그 이전 단계로 되돌린다.
      // currentDisplay == 1, currentDisplay == 3일때 메인 메뉴로 돌아가기
      if (currentDisplay == 1 || currentDisplay == 3) {
        currentDisplay = 0;
        drawMainMenu(lcd);
      }
    }
    
  }
  // z축을 누를 때
  else if (zValue == LOW) {
    delay(400);
    // 현재 메인 메뉴를 보고 있을 때에
    if (currentDisplay == 0) {
      // 첫번째 항목을 가리키고 있다면 0, 두번째 항목을 가리키고 있다면 1을 반환
      selectMode = cursorY;
      // 만약 selectMode == 0이라면 (시간을 수정하는 창으로 넘어가야 한다면)
      if (selectMode == 0) {
        // 시간 선택 메뉴를 보여주기
        drawChoiceTimeMenu(lcd);
        // 현재 페이지를 변경
        currentDisplay = 1;
      }
      // selectMode == 1이라면 (설정한 시간을 보여주는 창으로 넘어가야 한다면)
      else {
        // selectTime == 0, 시작 시간을 보여주기를 원한다면
        if (selectTime == 0) {
          //LCD 출력
          drawSettingTime(lcd);
        }
        // selectTime == 1, 종료 시간을 보여주기를 원한다면
        else if (selectTime == 1) {
          //LCD 출력
          drawSettingTime(lcd);
        }
        // 현재 페이지를 변경
        currentDisplay = 3;
      }
    }
    // currentDisplay == 1 : 현재 시간 선택 메뉴를 보여주고 있음
    else if (currentDisplay == 1) {
      // 시작 시간을 가리키고 있다면 0, 종료 시간을 가리키고 있다면 1을 반환
      selectTime = cursorY;
      // 시간 수정 창을 보여주기
      cursorX = leftX;
      if (selectTime == 0) {
        drawEditTimePage(lcd, startTime, cursorX);
      }
      else {
        drawEditTimePage(lcd, endTime, cursorX);
      }
      currentDisplay = 2;
    }
    // currentDisplay == 2 : 현재 시간을 수정하는 설정창을 보여주고 있음 
    else if (currentDisplay == 2) {
      lcd.clear();
      lcd.print("Time Changed");
      delay(3000);
      cursorX = 0;
      cursorY = 0;
      // 다시 메인 메뉴로 돌아간다
      drawMainMenu(lcd);
      currentDisplay = 0;
    }
    // currentDisplay == 3 : 현재 설정된 시간을 보여주고 있음
    else if (currentDisplay == 3) {
      // 다시 메인 메뉴로 돌아간다
      drawMainMenu(lcd);
      currentDisplay = 0;
    }
  }
}
