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
  virtual bool act() {return false;}
  char* getName() {return "no op"; };
} justSitThere;

class context : public behavior{
protected: 
  behavior* _currentBehavior;
  behavior* _defaultBehavior;
  virtual void beforeAct() {}
  virtual bool afterAct(bool currentBehaviorWantsToContinue) {
    if (!currentBehaviorWantsToContinue) {
      ensureBehavior(*_defaultBehavior);
    }
    return true;
  }
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
    return afterAct(shouldKeepCurrentBehavior);
  }
  void ensureBehavior(behavior& newBehavior) {
    if (&newBehavior == _currentBehavior) return;
    transitionTo(newBehavior);
  }
};

class behaviorSequence: public context {
  private:
  int _currentBehaviorIndex;
  bool _done;
  bool activateCurrentBehavior() {
    behavior* newBehavior = _behaviors[_currentBehaviorIndex];
    if (newBehavior == NULL) {
      Serial.print("current behavior sequence done");
      _done = true;
      return false;
    }
    transitionTo(*newBehavior);
    return true;
  }
  protected:
  behavior* _behaviors[10];
  virtual bool afterAct(bool currentBehaviorWantsToContinue) {
    if (_done) 
      return false;

    if (currentBehaviorWantsToContinue) 
      return true;

    ++ _currentBehaviorIndex;
    Serial.print(" xxx ");
    Serial.println(_currentBehaviorIndex);
    return activateCurrentBehavior();
  }
  public: 
  behaviorSequence(){
    _behaviors[0] = NULL;
  }
  virtual void activate () {
    _done = false;
    _currentBehaviorIndex = 0;
    
    context::activate();
    activateCurrentBehavior();
  }
};
