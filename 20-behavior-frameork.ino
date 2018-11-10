class behavior {
  protected: 
  unsigned long _since;
  public: 
  virtual void activate () {
    _since = millis();
  }
  // return whether this behavior should continue
  virtual bool act();
  virtual char* getName();
};
  
class justSitThereBehavior : public behavior{
  public:
  virtual bool act() {}
  char* getName() {return "no op"; };
} justSitThere;

class context : public behavior{
protected: 
  behavior* _currentBehavior;
  behavior* _defaultBehavior;
  virtual void beforeAct() {}
  virtual bool afterAct() {}
  void transitionTo(behavior& newBehavior) {
    Serial.print("transition to: ");
    Serial.println(newBehavior.getName());
    _currentBehavior = &newBehavior;
    newBehavior.activate();
  }
public:
  context() :
    _currentBehavior(&justSitThere),
    _defaultBehavior(&justSitThere) 
  {}
  virtual void activate() {
    transitionTo(*_defaultBehavior);
    behavior::activate();
  }
  virtual bool act() {
    beforeAct();
    bool shouldKeepCurrentBehavior = _currentBehavior->act();
    //todo move this if to after afterAct
    if (!shouldKeepCurrentBehavior) {
      ensureBehavior(*_defaultBehavior);
    }
    return afterAct();
  }
  void ensureBehavior(behavior& newBehavior) {
    if (&newBehavior == _currentBehavior) return;
    transitionTo(newBehavior);
  }
};