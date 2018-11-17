motor rightMotor(6, 9);
motor leftMotor(3, 5);

led yellowLed(11);
led redLed(12);
led greenLed(13);

analogReader proximityRaw(A5);
opponentVisibilityInterpreter opponentVisibility;
smoothInput proximity(proximityRaw, opponentVisibility, 50);

analogReader lightRearLeftRaw(A0);
opponentContactRearInterpreter opponentContactRearLeft;
smoothInput lightRearLeft(lightRearLeftRaw, opponentContactRearLeft, 30);

analogReader lightRearRightRaw(A4);
opponentContactRearInterpreter opponentContactRearRight;
smoothInput lightRearRight(lightRearRightRaw, opponentContactRearRight, 30);

reflectancePairReader reflectancePairRaw((unsigned char[]){7, 8});
reflectanceReader reflectanceLeftRaw(reflectancePairRaw, 0);
surfaceInterpreter surfaceLeft;
smoothInput reflectanceLeft(reflectanceLeftRaw, surfaceLeft, 250);
  
reflectanceReader reflectanceRightRaw(reflectancePairRaw, 1);
surfaceInterpreter surfaceRight;
smoothInput reflectanceRight(reflectanceRightRaw, surfaceRight, 250);

bool edgeDetected() {
  return surfaceLeft.getLatest() == SURFACE_EDGE || 
         surfaceRight.getLatest() == SURFACE_EDGE;
}

class turnBehavior: public behavior {
  private:
  int _speed;
  int _angleDegrees;
  unsigned long _millisecondsToComplete;
  public :
  turnBehavior(int speed, int angleDegrees):
    _speed(speed),
    _angleDegrees(angleDegrees)
  {
    unsigned long millisecondsToComplete360Turn = (unsigned long)1800 * motor::MAX_SPEED / speed;
    _millisecondsToComplete = millisecondsToComplete360Turn * abs(angleDegrees) / 360;
  }
  virtual void activate() {
    behavior::activate();
    int leftSpeedSign = _angleDegrees > 0? -1 : 1;
    leftMotor.setSpeed(leftSpeedSign * _speed);
    rightMotor.setSpeed(-leftMotor.getSpeed());
  }
  virtual bool act() {
    return millis() - _since < _millisecondsToComplete;
  }
  char* getName() {return "turn"; }
};

class scanBehavior: public behaviorSequence {
   turnBehavior quick360DegreeTurnRight;
   turnBehavior quick360DegreeTurnLeft;
  public: 
   scanBehavior(int angleDegrees, int speed = motor::MAX_SPEED):
     quick360DegreeTurnRight(speed, -angleDegrees),
     quick360DegreeTurnLeft(speed, angleDegrees){
    _behaviors[0] = (behavior*)(&quick360DegreeTurnRight);
    _behaviors[1] = (behavior*)(&quick360DegreeTurnLeft);
    _behaviors[2] = NULL;
  }
  virtual void activate () {
    greenLed.turnOn();
    yellowLed.turnOff();
    redLed.turnOff();
    if (leftMotor.getSpeed() < rightMotor.getSpeed()){
      _behaviors[0] = (behavior*)(&quick360DegreeTurnLeft);
      _behaviors[1] = (behavior*)(&quick360DegreeTurnRight);
    }
    else {
      _behaviors[0] = (behavior*)(&quick360DegreeTurnRight);
      _behaviors[1] = (behavior*)(&quick360DegreeTurnLeft);
    }
    behaviorSequence::activate();
  }
  char* getName() {return "scan"; }
};

scanBehavior wideScanFromUnknownPosition(360, motor::MAX_SPEED* 0.4);
scanBehavior narrowScan(20);

class backOffBehavior: public behavior {
public:
  virtual void activate() {
    behavior::activate();
    yellowLed.turnOnBlink();
    redLed.turnOff();

    if (surfaceRight.getLatest() == SURFACE_RING && 
      (surfaceLeft.getLatest() == SURFACE_EDGE ||
        surfaceRight.getSince() - surfaceLeft.getSince() > 250)
    ) {
      leftMotor.setSpeed(-motor::MAX_SPEED);
      rightMotor.setSpeed(-motor::MAX_SPEED * 0.5);
    } else if (surfaceLeft.getLatest() == SURFACE_RING && 
      (surfaceRight.getLatest() == SURFACE_EDGE ||
       surfaceLeft.getSince() - surfaceRight.getSince() > 250)
    ) {
      rightMotor.setSpeed(-motor::MAX_SPEED);
      leftMotor.setSpeed(-motor::MAX_SPEED * 0.5);
    }
  }
  virtual bool act() {
    unsigned long now = millis();
    if (surfaceRight.getLatest() == SURFACE_RING && 
      surfaceLeft.getLatest() == SURFACE_RING &&
      now - surfaceLeft.getSince() > 150 &&
      now - surfaceRight.getSince() > 150) {
      leftMotor.setSpeed(-motor::MAX_SPEED);
      rightMotor.setSpeed(-motor::MAX_SPEED);
    }
    unsigned long _millisecondsToBackOffAfterGettingOnRing = 850;
    return edgeDetected() ||
        now - surfaceLeft.getSince() < _millisecondsToBackOffAfterGettingOnRing ||
        now - surfaceRight.getSince() < _millisecondsToBackOffAfterGettingOnRing;
  }
  char* getName() {return "back off"; };
};

class fightStartBehavior: public behavior {
public:
  virtual void activate() {
    behavior::activate();
    yellowLed.turnOn();
    redLed.turnOff();
    randomSeed(millis());
    bool turnRight = (random(2) == 0);
    rightMotor.setSpeed(turnRight? 0: motor::MAX_SPEED);
    leftMotor.setSpeed(turnRight? motor::MAX_SPEED: 0);
  }
  virtual bool act() {
    return millis() - _since < 1000;
  }
  char* getName() {return "fight start"; };
};

class attackBehavior: public behavior {
private:
  int _tieBreakLeftSpeed;
public:
  virtual void activate() {
    behavior::activate();
    yellowLed.turnOff();
    greenLed.turnOff();
    leftMotor.setSpeed(motor::MAX_SPEED);
    rightMotor.setSpeed(motor::MAX_SPEED);
    _tieBreakLeftSpeed = random(motor::MAX_SPEED* 0.33, motor::MAX_SPEED);
  }
  virtual bool act() {
    if (millis() - _since > 20000) { 
      leftMotor.setSpeed(-_tieBreakLeftSpeed);
      rightMotor.setSpeed(-motor::MAX_SPEED / 2);
      redLed.turnOnBlink();
    }
    else {
      redLed.turnOn();
    }
    return opponentVisibility.getLatest();
  }
  char* getName() {return "attack"; };
};

//todo derive from context
class evadeAndCircleBackBehavior: public behavior {
  private:
  turnBehavior quickTurnRight;
  turnBehavior quickTurnLeft;
  behavior* _currentBehavior;
  public:
  virtual void activate() {
    behavior::activate();
    redLed.turnOnBlink();
    yellowLed.turnOnBlink();
    greenLed.turnOff();
    
    _currentBehavior = opponentContactRearRight.getLatest() ? 
      &quickTurnLeft :
      &quickTurnRight;
    _currentBehavior->activate();
  }

  evadeAndCircleBackBehavior():
    quickTurnRight(motor::MAX_SPEED, -360),
    quickTurnLeft(motor::MAX_SPEED, 360){
      _currentBehavior = &justSitThere;
  }
  virtual bool act() {
    _currentBehavior->act();
    //todo: extract method
    // confirmLatestValueForPastMilliseconds(T value, 
    //                                       unsigned long milliseconds)
    unsigned long now = millis();
    unsigned long timeItTakesToCompleteManeuver = 10000; //time it takes to turn 306 degrees?
    //todo: create separate method behavior::shouldContinue
    bool  shouldContinue = 
      opponentContactRearLeft.getLatest() || opponentContactRearRight.getLatest() ||
      now - opponentContactRearLeft.getSince() < timeItTakesToCompleteManeuver ||
      now - opponentContactRearRight.getSince() < timeItTakesToCompleteManeuver;
    return shouldContinue;
  }
  virtual bool shouldUrgentlyAct() {
    return 
      opponentContactRearLeft.getLatest() ||
      opponentContactRearRight.getLatest();
  }
  char* getName() {return "evadeAndCircleBack"; };
};

class preFightBehavior : public behavior{
public:
  virtual void activate() {
    behavior::activate();      
    yellowLed.turnOnBlink();
    redLed.turnOnBlink();
    greenLed.turnOnBlink();
    leftMotor.setSpeed(0);
    rightMotor.setSpeed(0);
  }
  virtual bool act() { 
    const unsigned long PRE_FIGHT_DELAY = 5000;
    unsigned long now = millis();
    bool shouldContinue = 
      now - surfaceLeft.getSince() < PRE_FIGHT_DELAY && 
      now - surfaceRight.getSince() < PRE_FIGHT_DELAY;
    return shouldContinue;
  }
  char* getName() {return "pre fight"; };
};

class freezeAllMotorFunctionsBehavior : public behavior{
  private: 
  bool _sent;
public:
  freezeAllMotorFunctionsBehavior() : _sent(false) {}
  virtual void activate() {
    _sent = false;
    behavior::activate();
    leftMotor.setSpeed(0);
    rightMotor.setSpeed(0);
    redLed.turnOff();
    yellowLed.turnOff();
    greenLed.turnOff();
  }
  virtual bool act() { 
    if (!_sent && Serial) {
      Serial.println("todo send all logs here");
      _sent = true;
    }
    return true;
  }
  char* getName() {return "freezeAllMotorFunctions"; };
};

preFightBehavior preFight;
fightStartBehavior fightStart;
attackBehavior attack;
backOffBehavior backOff;
evadeAndCircleBackBehavior evadeAndCircleBack;

freezeAllMotorFunctionsBehavior freezeAllMotorFunctions;


class fightContext: public context {
  protected: 
    virtual void beforeAct () {
      if (_currentBehavior == &preFight)
        return;
      //todo: if backOff.sholdUrgentlyAct()
      if (edgeDetected())
        ensureBehavior(backOff);
      //todo: if attack.sholdUrgentlyAct()
      else if (opponentVisibility.getLatest())
        ensureBehavior(attack);
      else if (!opponentVisibility.getLatest() && 
        millis() - opponentVisibility.getSince() < 200) {
        ensureBehavior(narrowScan);
        redLed.turnOn();
      }
      else if (evadeAndCircleBack.shouldUrgentlyAct()) {
        ensureBehavior(evadeAndCircleBack);
      }
    }
  
  public:
  void activate () {
    _defaultBehavior = &wideScanFromUnknownPosition;
    transitionTo(preFight);
    behavior::activate();
  }  
  char* getName() {return "fight"; };
};

class debugContext : public context{
  public:
  void activate () {
    _defaultBehavior = &freezeAllMotorFunctions;
    context::activate();
  }
  char* getName() {return "debug"; };
};

fightContext fight;
debugContext debug;

class sumoBotContext : public context{
  protected: 
    virtual void beforeAct () {
      unsigned long now = millis();
      if (
        surfaceLeft.getLatest() == SURFACE_EDGE &&
        surfaceRight.getLatest() == SURFACE_EDGE && 
        opponentVisibility.getLatest())
      {
        ensureBehavior(debug);
      } else if (
        surfaceLeft.getLatest() == SURFACE_RING &&
        surfaceRight.getLatest() == SURFACE_RING && 
        now - surfaceLeft.getSince() >= 150 && 
        now - surfaceRight.getSince() >= 150)
      {
          ensureBehavior(fight);
      }
    }
  
  public:
  void activate () {
    _defaultBehavior = &debug;
    context::activate();
  }
  char* getName() {return "sumo bot"; };
};
  

sumoBotContext sumoBot;
  
void setup(){
  Serial.begin(9600);
  yellowLed.init();
  redLed.init();
  greenLed.init();
  leftMotor.init();
  rightMotor.init();
      
  redLed.turnOnBlink();
  sumoBot.activate();
}

void loop() {
  proximity.updateInput();
  reflectanceLeft.updateInput();
  reflectanceRight.updateInput();
  lightRearLeft.updateInput();
  lightRearRight.updateInput();
  
  sumoBot.act();
  
  //todo: motors.updateOutput?
  yellowLed.updateOutput();
  redLed.updateOutput();
  greenLed.updateOutput();
}