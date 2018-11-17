class motor {
  private:
  int _pin1;
  int _pin2;
  int _speed;

  public:
  static const int MAX_SPEED = 255;
  motor(int pin1, int pin2) : 
    _pin1(pin1),
    _pin2(pin2)
  {}

  void init() {
    pinMode(_pin1, OUTPUT);
    pinMode(_pin2, OUTPUT);
  }
  void setSpeed(int speed) {
    _speed = speed;
    if (speed < 0) {
      analogWrite(_pin1, -speed);
      analogWrite(_pin2, 0);
    } 
    else {
      analogWrite(_pin1, 0);
      analogWrite(_pin2, speed);
    }
  }
  int getSpeed() {
    return _speed;
  }
};