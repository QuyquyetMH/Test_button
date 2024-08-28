#include "ButtonControl.h"

ButtonControl::ButtonControl(int upPin, int downPin, int pausePin, int startPin): _upPin(upPin), _downPin(downPin), _pausePin(pausePin), _startPin(startPin), countTime(countTime), _isPaused(false) {}

void ButtonControl::begin() {
    pinMode(_upPin, INPUT_PULLUP);
    pinMode(_downPin, INPUT_PULLUP);
    pinMode(_pausePin, INPUT_PULLUP);
    pinMode(_startPin, INPUT_PULLUP);
}

// void ButtonControl::handleButtonPress() {
//     if (digitalRead(_upPin) == LOW) {
//         _upButtonPressed();
//         delay(200);  // Debounce delay
//     }
//     if (digitalRead(_downPin) == LOW) {
//         _downButtonPressed();
//         delay(200);  // Debounce delay
//     }
//     if (digitalRead(_pausePin) == LOW) {
//         _pauseButtonPressed();
//         delay(200);  // Debounce delay
//     }
//     if (digitalRead(_startPin) == LOW) {
//         _startButtonPressed();
//         delay(200);  // Debounce delay
//     }
// }

// void ButtonControl::_upButtonPressed() {
//     if (!_isPaused) {
//         countTime += 500;
//     }
// }

// void ButtonControl::_downButtonPressed() {
//     if (!_isPaused) {
//         countTime -= 500;
//     }
// }

// void ButtonControl::_pauseButtonPressed() {
//     _isPaused = !_isPaused;
// }

// void ButtonControl::_startButtonPressed() {
//     if (_isPaused) {
//         _isPaused = false;
//     }
//     // Add logic here to start or resume the main program if needed.
// }

// bool ButtonControl::isPaused() {
//     return _isPaused;
// }