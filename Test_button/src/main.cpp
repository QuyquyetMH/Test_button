#include <Arduino.h>
#include <EEPROM.h>
#include <lcd.h>
// #include <Preferences.h>
#define BUZZER_PIN     12 
#define MOSFET_PIN     2
#define SENSOR_PIN     4  // Chân GPIO4 của ESP32 kết nối với ngõ ra của sensor
#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

LCD lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);
/****************************************************
 *                    EEPROM                        *
 ***************************************************/
#define PRESS_COUNT_ADDR 0
#define SETUP_COUNT_ADDR 4

/***************************************************
 *                  CHECK JIGS                      *
 ****************************************************/
int ValveState;
int sensorState = 0;      // Biến để lưu trữ trạng thái của sensor
void Checksensor(){
sensorState = digitalRead(SENSOR_PIN);  // Đọc trạng thái của sensor
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
  // Tắt MOSFET (tải ngừng hoạt động
    digitalWrite(MOSFET_PIN, LOW);
      ValveState == 0;
    delay(4000);  // Duy trì trạng thái OFF trong 4 giây
        Checksensor();
}
/***************************************************
*               CHECK BUTTON                        *
****************************************************/
int8_t buttonState = 0;      // Biến để lưu trạng thái hiện tại của button
int8_t errorCode = 0; // Lưu biến lỗi
int32_t SetupCount = 0;        // Biến setup số lần nhấn
int8_t errorCount2 = 0;     // Biến đếm số lần lỗi liệt nút
int8_t errorCount3 = 0;     // Biến đếm số lần lỗi dính nút
int32_t pressCount = 0;    

bool systemRunning;
volatile bool isPaused = false;  // Biến dùng để lưu trạng thái của hệ thống

#define BUTTON_PIN   34
#define BUTTON_UP_PIN 25
#define BUTTON_DOWN_PIN 26
#define BUTTON_PAUSE_PIN 27
#define BUTTON_START_PIN 32


/****************************************************
*****************************************************/
typedef enum DropButton{
     BUTTON_START,
     BUTTON_PAUSE,
     BUTTON_UP,
     BUTTON_DOWN
};
    
  DropButton ButtonStatus = BUTTON_PAUSE;

    void Checkbutton(){
       if(digitalRead(BUTTON_PAUSE_PIN) ==LOW){
        buttonState = BUTTON_PAUSE;
       }
       else if(digitalRead(BUTTON_START_PIN)==LOW){
          buttonState = BUTTON_START;
       }
            else if(digitalRead(BUTTON_DOWN_PIN)==LOW){
              buttonState = BUTTON_DOWN;
            }
                 if(digitalRead(BUTTON_UP_PIN)==LOW){
                    buttonState = BUTTON_UP;
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
    RUNNING_PITON,
    CHECK_BUTTON_1,
    CHECK_BUTTON_2,
    CLOSE_PITON,
    PAUSE_SYSTEM,
    SYSTEM_ERROR_1,
    SYSTEM_ERROR_2,
    SYSTEM_ERROR_3 
};

SystemState currentState = PAUSE_SYSTEM; // Khởi tạo trạng thái hiện tại


void StateManager(){
    switch (eventCurrentState)
    {
    case STATE_EVENT_STARTUP:
         lcd.displayStatus("SETUP");
         Checkbutton();
         switch (buttonState)
         {
         case BUTTON_PAUSE:
             EEPROM.write(PRESS_COUNT_ADDR,pressCount);
             EEPROM.commit();
          break;
         case BUTTON_START:
             eventCurrentState = STATE_EVENT_IDLE;
          break;
          case BUTTON_UP:
             SetupCount +=500;
             buttonState = BUTTON_PAUSE;
          break;
          case BUTTON_DOWN:
             SetupCount -=500;
             buttonState = BUTTON_PAUSE;
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
            case RUNNING_PITON:
                CloseValve();
                if (ValveState != sensorState){
                 currentState = SYSTEM_ERROR_1;
                }
                else currentState = CHECK_BUTTON_1;
                break;
                
            case CHECK_BUTTON_1:
                  buttonState = digitalRead(BUTTON_PIN);
                if (buttonState == sensorState){
                    errorCount2 = 0;
                    pressCount ++;
                    lcd.displayCounts(pressCount, SetupCount);
                    currentState = CLOSE_PITON;
                }
                else 
                   currentState = SYSTEM_ERROR_2;
                break;
            case CLOSE_PITON:
                OpenValve ();
                if (ValveState != sensorState){
                 currentState = SYSTEM_ERROR_1;
                }
                else currentState = CHECK_BUTTON_2 ;
                break;
                
            case CHECK_BUTTON_2:
                 buttonState = digitalRead(BUTTON_PIN);
                 if(buttonState == sensorState){
                  errorCount3 = 0;
                  currentState = RUNNING_PITON;
                 }
                 else 
                   currentState = SYSTEM_ERROR_3;
            break;
            
            case PAUSE_SYSTEM:
                 lcd.displayStatus("PAUSE");
                 digitalWrite(MOSFET_PIN, LOW);
                 EEPROM.write(PRESS_COUNT_ADDR,pressCount);
                 EEPROM.commit();
                break;
                
            case SYSTEM_ERROR_1:
                currentState = PAUSE_SYSTEM;
                errorCode = 1;
                break;
                
            case SYSTEM_ERROR_2:
                 errorCount2 ++;
                if (errorCount2 > 5){
                     errorCode = 2;
                    currentState = PAUSE_SYSTEM;
                }
                else 
                    currentState = RUNNING_PITON;
                break;
            case  SYSTEM_ERROR_3 :
                  errorCount3 ++;
                if (errorCount3 > 5){
                    errorCode = 3;
                  currentState = PAUSE_SYSTEM;
                }
                else 
                    currentState = CLOSE_PITON;
                
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
  pinMode(SENSOR_PIN, INPUT);      // Thiết lập chân GPIO4 là INPUT
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