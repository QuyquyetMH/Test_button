#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
//#include <Preferences.h>
#include <LiquidCrystal_I2C.h>
//#include <lcd.h>
#define BUZZER_PIN     12 
#define MOSFET_PIN     2
#define Sensor_Pin     4  // Chân GPIO4 của ESP32 kết nối với ngõ ra của sensor
LiquidCrystal_I2C lcd(0x27, 20, 4);
/****************************************************
 *                    EEPROM                        *
 ***************************************************/
#define PRESS_COUNT_ADDR 0
#define SETUP_COUNT_ADDR 4
   // Biến để đếm số lần button được nhấn

/***************************************************
 *                  CHECK JIGS                      *
 ****************************************************/
int ValveState;
int sensorState = 0;      // Biến để lưu trữ trạng thái của sensor
void Checksensor(){
sensorState = digitalRead(Sensor_Pin);  // Đọc trạng thái của sensor
  if (sensorState == HIGH) {
      sensorState = 0;
  } 
  else {
    sensorState = 1;
    Serial.print("ValveState: ");Serial.print(ValveState);
    Serial.print("sensorState: ");Serial.print(sensorState);
       }
  delay(500);  
}
void CloseValve(){
      digitalWrite(MOSFET_PIN, LOW);
      ValveState = 1;
      Serial.print(ValveState);
    delay(4000);  // Duy trì trạng thái ON trong 4 giây
        Checksensor();
  
}
void OpenValve(){
  // Tắt MOSFET (tải ngừng hoạt động)
    digitalWrite(MOSFET_PIN, HIGH);
      ValveState = 0;
      Serial.print(ValveState);
    delay(4000);  // Duy trì trạng thái OFF trong 4 giây
        Checksensor();
}
/***************************************************
*               CHECK BUTTON                        *
****************************************************/
int buttonState = 0;      // Biến để lưu trạng thái hiện tại của button
int errorCode = 0;        // Lưu biến lỗi
int SetupCount = 500;     // Biến setup số lần nhấn
int errorCount2 = 0;      // Biến đếm số lần lỗi liệt nút
int errorCount3 = 0;      // Biến đếm số lần lỗi dính nút
int pressCount = 0;    

#define BUTTON_PIN   34
#define BUTTON_UP_PIN 14
#define BUTTON_DOWN_PIN 27
#define BUTTON_PAUSE_PIN 26
#define BUTTON_START_PIN 25


/****************************************************
*****************************************************/
typedef enum DropButton{
     ButtonStart,
     ButtonPause,
     ButtonUp,
     ButtonDown
};
    
  DropButton ButtonStatus = ButtonPause;

    void Checkbutton(){
       if(digitalRead(BUTTON_PAUSE_PIN) ==LOW){
        buttonState = ButtonPause;
       }
       else if(digitalRead(BUTTON_START_PIN)==LOW){
          buttonState = ButtonStart;
       }
            else if(digitalRead(BUTTON_DOWN_PIN)==LOW){
              buttonState = ButtonDown;
            }
                 if(digitalRead(BUTTON_UP_PIN)==LOW){
                    buttonState = ButtonUp;
                  }
    }
/*********************************************************
 *                     OPERATION SYSTEM                  *
 ********************************************************/
typedef enum EventState{
  STATE_EVENT_STARTUP,
  STATE_EVENT_IDLE,
  STATE_EVENT_RETURN
};
EventState eventCurrentState = STATE_EVENT_STARTUP;

typedef enum SystemState {
    RunningPiton,
    CheckButton1,
    CheckButton2,
    ClosePiton,
    PauseSystem,
    SystemError1,
    SystemError2,
    SystemError3 
};

SystemState currentState = RunningPiton; // Khởi tạo trạng thái hiện tại


void StateManager(){
    lcd.setCursor(0, 0); // Đặt con trỏ tại hàng đầu tiên, cột đầu tiên
    lcd.print("pressCount: ");lcd.print(pressCount);
    // Hiển thị "SetupPress" với giá trị của biến countTime
    lcd.setCursor(0, 1); // Đặt con trỏ tại hàng thứ hai, cột đầu tiên
    lcd.print("SetupPress: ");
    lcd.print(SetupCount);

    
    switch (eventCurrentState)
    {
    case STATE_EVENT_STARTUP:
         Serial.print("setup_____");
         lcd.setCursor(0,2);
         lcd.print("SETUP");
         Checkbutton();
         switch (buttonState)
         {
         case ButtonPause:
         Serial.print("PAUSE_____");
         lcd.setCursor(0,2);
         lcd.print("PAUSE");
             EEPROM.write(PRESS_COUNT_ADDR,pressCount);
             EEPROM.commit();
          break;
         case ButtonStart:
                  Serial.print("RUN_____");
                  lcd.setCursor(0,2);
                  lcd.print("         ");
                  lcd.setCursor(0,2);
                  lcd.print("RUN");
             eventCurrentState = STATE_EVENT_IDLE;
          break;
          case ButtonUp:
             SetupCount +=500;
             buttonState = ButtonPause;
          break;
          case ButtonDown:
             SetupCount -=500;
             buttonState = ButtonPause;
          break;
         }
    break;

    case STATE_EVENT_IDLE:
      if(pressCount <= SetupCount)
          OperationSystem();
      else {
        eventCurrentState = STATE_EVENT_STARTUP;
      }
    break;


    case STATE_EVENT_RETURN:
           eventCurrentState = STATE_EVENT_STARTUP;
    break;

    default:
      break;
    }
}
void OperationSystem() {
        switch(currentState) {
            case RunningPiton:
                CloseValve();
                if (ValveState != sensorState){
                 currentState = SystemError1;
                }
                else currentState = CheckButton1;
                break;
                
            case CheckButton1:
                  buttonState = digitalRead(BUTTON_PIN);
                if (buttonState == sensorState){
                    errorCount2 = 0;
                    pressCount ++;
                    Serial.print("HHHHHHH");
                    currentState = ClosePiton;
                }
                else 
                   currentState = SystemError2;
                break;
            case ClosePiton:
                OpenValve ();
                Serial.print("??????");
                if (ValveState != sensorState){
                 currentState = SystemError1;
                }
                else currentState = CheckButton2 ;
                break;
                
            case CheckButton2:
                 buttonState = digitalRead(BUTTON_PIN);
                 if(buttonState == sensorState){
                  errorCount3 = 0;
                  currentState = RunningPiton;
                 }
                 else 
                   currentState = SystemError2;
            break;
            
            case PauseSystem:
                 Serial.print("<<<>>>>");
                 digitalWrite(MOSFET_PIN, LOW);
              if(digitalRead(BUTTON_START_PIN)==LOW){
                 currentState = RunningPiton;
                 EEPROM.write(PRESS_COUNT_ADDR,pressCount);
                 EEPROM.commit();
              }
                break;
                
            case SystemError1:
                currentState = PauseSystem;
                errorCode = 1;
                break;
                
            case SystemError2:
                 errorCount2 ++;
                if (errorCount2 > 5){
                     errorCode = 2;
                    currentState = PauseSystem;
                }
                else 
                    currentState = RunningPiton;
                break;
            case SystemError3:
                  errorCount3 ++;
                if (errorCount3 > 5){
                    errorCode = 3;
                  currentState = PauseSystem;
                }
                else 
                    currentState = ClosePiton;
                
                break;
        }
    }
/**********************************************************/
 /****************************************************** */
void setup() {
  Serial.begin(115200);             // Khởi tạo Serial Monitor
  pinMode(BUTTON_PIN, INPUT);       // Thiết lập chân GPIO34 là INPUT để đọc trạng thái button
  pinMode(MOSFET_PIN, OUTPUT);      // Khởi tạo chân GPIO điều khiển MOSFET là OUTPUT
  digitalWrite(MOSFET_PIN, LOW);    // Đặt ban đầu MOSFET ở trạng thái OFF
  pinMode(Sensor_Pin, INPUT);      // Thiết lập chân GPIO4 là INPUT
    pinMode(BUTTON_UP_PIN, INPUT_PULLUP);
    pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PAUSE_PIN, INPUT_PULLUP);
    pinMode(BUTTON_START_PIN, INPUT_PULLUP);
     lcd.clear();
     lcd.init();
     lcd.backlight();
    pressCount = EEPROM.read(PRESS_COUNT_ADDR);
  //Preferences.begin("Tool", false);

    // Đọc giá trị pressCount từ Flash (nếu có)
  //pressCount = Preferences.getInt("pressCount", 0); // Đọc giá trị, mặc định là 0 nếu không tồn tại
}


void loop() {
  Serial.print("Start\n");
  StateManager();
}