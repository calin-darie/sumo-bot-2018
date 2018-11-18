class motor {
  private:
  int _pin1;
  int _pin2;
  int _speed;
  int _requestedSpeed;
  int _speedRequestedSince;
  static const unsigned long MILLISECONDS_TO_TRANSITION = 70;

  public:
  static const int MAX_SPEED = 255;
  static const unsigned long MILLISECONDS_TO_SET_SPEED = 
    2 * MILLISECONDS_TO_TRANSITION;
  motor(int pin1, int pin2) : 
    _pin1(pin1),
    _pin2(pin2)
  {}

  void init() {
    pinMode(_pin1, OUTPUT);
    pinMode(_pin2, OUTPUT);
  }
  void setSpeed(int speed) {
    _speedRequestedSince = millis();
    _requestedSpeed = speed;
    _speed = (_requestedSpeed + _speed) / 2;
  }
  void updateOutput() {
    if (millis() - _speedRequestedSince > MILLISECONDS_TO_TRANSITION) {
      _speed = _requestedSpeed;
    }
    if (_speed < 0) {
      analogWrite(_pin1, -_speed);
      analogWrite(_pin2, 0);
    } 
    else {
      analogWrite(_pin1, 0);
      analogWrite(_pin2, _speed);
    }

  }
  int getSpeed() {
    return _speed;
  }
};