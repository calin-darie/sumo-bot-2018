enum LED_STATE {
  LED_OFF,
  LED_ON, 
  LED_BLINK
};

class led {
  private:
  int _pin;
  bool _isOn;
  LED_STATE _desiredState;
  void setIsOnAndWrite(bool value) {
    if (_isOn == value) return;
    digitalWrite(_pin, value);
    _isOn = value;
  }
  public: 
  led(int pin): _pin(pin) { }
  void init () {
    pinMode(_pin, OUTPUT);
  }
  void turnOnBlink() {
    _desiredState = LED_BLINK;
  }
  void turnOn() {
    _desiredState = LED_ON;
  }
  void turnOff() {
    _desiredState = LED_OFF;
  }
  void updateOutput() {
    switch (_desiredState) {
      case LED_ON:
        setIsOnAndWrite(true);
        break;
      case LED_OFF:
        setIsOnAndWrite(false);
        break;
      case LED_BLINK:
        setIsOnAndWrite(millis() % 1000 < 500);
        break;
    }
  }
};
