#ifndef BUTTONCONTROL_H
#define BUTTONCONTROL_H

#include <Arduino.h>

class ButtonControl {
public:
    ButtonControl(int upPin, int downPin, int pausePin, int startPin);
    void begin();
    // void handleButtonPress();
    // void triggerError(); 
    // bool isPaused();    
private:
    int _upPin;
    int _downPin;
    int _pausePin;
    int _startPin;
    int countTime;
    volatile bool _isPaused;
    
    void _upButtonPressed();
    void _downButtonPressed();
    void _pauseButtonPressed();
    void _startButtonPressed();
};

#endif // BUTTONCONTROL_H
