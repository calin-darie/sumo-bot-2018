
class reader {
public:
virtual int read() const;
};


class iInputInterpreter { 
  public: 
  virtual void handle(int input);
};

template<typename TOutput>
class inputInterpreter : public iInputInterpreter{ 
  private:
  TOutput _latestOutput;
  unsigned long _since;
  protected:
  virtual TOutput convert(int input);

  public:
  void handle(int input) {
    TOutput currentOutput = convert(input);
    if (_latestOutput != currentOutput) {
      _latestOutput = currentOutput;
      _since = millis();
  }
}
TOutput getLatest() {
  return _latestOutput;
}
unsigned long getSince() {
  return _since;
}
};

class smoothInput {
  private:
  static const int numReadings = 10;
  static const int throttleTimeMs = 75;

  int _readings[numReadings];
  int _currentReadIndex;
  int _runningTotal;
  int _precision;
  int _latestValue;
  unsigned long _lastEmitTime;
  bool _enoughData;
  const reader& _reader;
  iInputInterpreter& _interpreter;

  int _maxProposedPrecision;

  static int roundToPrecision(int value, int precision) {
    return (value + precision / 2) / precision * precision;
  }

  inline int runningAverage() {
    return _runningTotal / numReadings;
  }

  void advanceReadIndex() {
    ++ _currentReadIndex;
    if (_currentReadIndex  >= numReadings ) {
      _currentReadIndex = 0;
      _enoughData = true;
    }
  }

  void setLatestValue(int value) {
    if (abs(_latestValue - value) < _precision/2) return;
    unsigned long now = millis();
    if (now - _lastEmitTime < throttleTimeMs) return;
    
    _latestValue = value;
    _interpreter.handle(roundToPrecision(_latestValue, _precision));
    _lastEmitTime = now;      
  }

  public: 
  smoothInput(const reader& reader, iInputInterpreter& interpreter, int precision): 
    _reader(reader),
    _interpreter(interpreter),
    _precision(precision),
    _readings(),
    _currentReadIndex(0),
    _runningTotal(0),
    _latestValue(-precision),
    _lastEmitTime(-throttleTimeMs),
    _enoughData(false),
    _maxProposedPrecision(0)
  { };

  void updateInput() {
    _runningTotal -= _readings[_currentReadIndex];
    _readings[_currentReadIndex] = _reader.read();
    _runningTotal += _readings[_currentReadIndex];
    advanceReadIndex();
    
    if (!_enoughData) return;
    setLatestValue(runningAverage());
  }


  int calibratePrecision() {//todo: move to its own class?
    updateInput();

    _precision = 1;
    long sumOfSquaredDifferences = 0;
    for (int i = 0; i < numReadings; ++i) {
      int difference = _readings[i] - runningAverage();
      sumOfSquaredDifferences += (long)difference * difference;
    }
    int deviation = sqrt(sumOfSquaredDifferences / numReadings);
    int proposedPrecision = 2* deviation;
    if (proposedPrecision > _maxProposedPrecision) {
      _maxProposedPrecision = proposedPrecision;
      Serial.println(_maxProposedPrecision);
    }
  }
};