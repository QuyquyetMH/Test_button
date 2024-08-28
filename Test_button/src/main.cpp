#include <Arduino.h>
#include <EEPROM.h>
#include <lcd.h>
// #include <Preferences.h>
#define BUZZER_PIN     12 
#define MOSFET_PIN     2
#define Sensor_Pin     4  // Chân GPIO4 của ESP32 kết nối với ngõ ra của sensor
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

LCD lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
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
    Serial.printf("ValveState:",ValveState);
       }
  delay(500);  
}
void CloseValve(){
      digitalWrite(MOSFET_PIN, HIGH);
      ValveState = 1;
    delay(4000);  // Duy trì trạng thái ON trong 4 giây
        Checksensor();
  
}
void OpenValve(){
  // Tắt MOSFET (tải ngừng hoạt động)
    digitalWrite(MOSFET_PIN, LOW);
      ValveState == 0;
    delay(4000);  // Duy trì trạng thái OFF trong 4 giây
        Checksensor();
}
/***************************************************
*               CHECK BUTTON                        *
****************************************************/
int buttonState = 0;      // Biến để lưu trạng thái hiện tại của button
int errorCode = 0; // Lưu biến lỗi
int SetupCount = 0;        // Biến setup số lần nhấn
int errorCount2 = 0;     // Biến đếm số lần lỗi liệt nút
int errorCount3 = 0;     // Biến đếm số lần lỗi dính nút
int pressCount = 0;    

bool systemRunning;
volatile bool isPaused = false;  // Biến dùng để lưu trạng thái của hệ thống

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

SystemState currentState = PauseSystem; // Khởi tạo trạng thái hiện tại


void StateManager(){
    switch (eventCurrentState)
    {
    case STATE_EVENT_STARTUP:
         lcd.displayStatus("SETUP");
         Checkbutton();
         switch (buttonState)
         {
         case ButtonPause:
             EEPROM.write(PRESS_COUNT_ADDR,pressCount);
             EEPROM.commit();
          break;
         case ButtonStart:
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
        lcd.displayStatus("Please Setup");
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
                    lcd.displayCounts(pressCount, SetupCount);
                    currentState = ClosePiton;
                }
                else 
                   currentState = SystemError2;
                break;
            case ClosePiton:
                OpenValve ();
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
                 lcd.displayStatus("PAUSE");
                 digitalWrite(MOSFET_PIN, LOW);
                 EEPROM.write(PRESS_COUNT_ADDR,pressCount);
                 EEPROM.commit();
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
    lcd.begin();
    pressCount = EEPROM.read(PRESS_COUNT_ADDR);
  //Preferences.begin("Tool", false);

    // Đọc giá trị pressCount từ Flash (nếu có)
  //pressCount = Preferences.getInt("pressCount", 0); // Đọc giá trị, mặc định là 0 nếu không tồn tại
}


void loop() {
  StateManager();
}