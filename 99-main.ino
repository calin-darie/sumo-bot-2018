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
  
reflectanceReader reflectanceLeftRaw(0);
surfaceInterpreter surfaceLeft;
smoothInput reflectanceLeft(reflectanceLeftRaw, surfaceLeft, 250);
  
reflectanceReader reflectanceRightRaw(1);
surfaceInterpreter surfaceRight;
smoothInput reflectanceRight(reflectanceRightRaw, surfaceRight, 250);

bool edgeDetected() {
  return surfaceLeft.getLatest() == SURFACE_EDGE || 
          surfaceRight.getLatest() == SURFACE_EDGE;
}

class backOffBehavior: public behavior {
public:
  virtual void activate() {
    behavior::activate();
    yellowLed.turnOnBlink();
    redLed.turnOff();
  }
  virtual bool act() {
    //todo: move motors slightly out of sync to correct the position in case the bot is not perpendicular to the edge, otherwise full speed behind
    unsigned long now = millis();
    return edgeDetected() ||
        now - surfaceLeft.getSince() < 2500 ||
        now - surfaceRight.getSince() < 2500;
  }
  char* getName() {return "back off"; };
};

class fightStartBehavior: public behavior {
public:
  virtual void activate() {
    behavior::activate();
    yellowLed.turnOn();
    redLed.turnOff();
  }
  virtual bool act() {
    //todo: move motors
    return millis() - _since < 5000;
  }
  char* getName() {return "fight start"; };
};


class attackBehavior: public behavior {
public:
  virtual void activate() {
    behavior::activate();
    redLed.turnOn();
    yellowLed.turnOff();
  }
  virtual bool act() {
    //todo: move motors
    return opponentVisibility.getLatest();
  }
  char* getName() {return "attack"; };
};

class evadeAndCircleBackBehavior: public behavior {
public:
  virtual void activate() {
    behavior::activate();
    redLed.turnOnBlink();
    yellowLed.turnOnBlink();
    greenLed.turnOff();
  }
  virtual bool act() {
    //todo: move motors

    //todo: extract method
    // confirmLatestValueForPastMilliseconds(T value, 
    //                                       unsigned long milliseconds)
    unsigned long now = millis();
    unsigned long timeItTakesToCompleteManeuver = 4000; //time it takes to turn 306 degrees?
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
    //todo: stop all motors
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
      else if (evadeAndCircleBack.shouldUrgentlyAct()) {
        ensureBehavior(evadeAndCircleBack);
      }
    }
  
  public:
  void activate () {
    _defaultBehavior = &fightStart;
    context::activate();
    ensureBehavior(preFight);
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
        surfaceLeft.getLatest() == SURFACE_NONE ||
        surfaceRight.getLatest() == SURFACE_NONE)
      {
        ensureBehavior(debug);
      } else if (
        surfaceLeft.getLatest() == SURFACE_RING &&
        surfaceRight.getLatest() == SURFACE_RING && 
        now - surfaceLeft.getSince() > 500 && 
        now - surfaceRight.getSince() > 500)
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
  
  yellowLed.updateOutput();
  redLed.updateOutput();
  greenLed.updateOutput();
}
