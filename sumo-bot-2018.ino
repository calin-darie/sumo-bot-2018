  #define PIN_YELLOW 11
  #define PIN_RED 12
  #define PIN_GREEN 13

  #define IN_LIGHT_REARLEFT 0
  #define IN_LIGHT_REARRIGHT 4

  class reader {
    public:
    virtual int read() const;
  };

  class analogReader: public reader {
    const int inputPin;
    public: 
    analogReader(int inputPin) : 
      inputPin(inputPin)
    {}
    
    int read() const{
      return analogRead(inputPin);
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
    const reader& _reader;
    bool _enoughData;
    
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
      if (abs(_latestValue - value) < _precision) return;
      //onValueChanged(latestValue, value);
      unsigned long now = millis();
      if (now - _lastEmitTime < throttleTimeMs) return;
      
      _latestValue = value;
      Serial.println(roundToPrecision(value, _precision));
      _lastEmitTime = now;      
    }
    
    public: 
    smoothInput(const reader& reader, int precision): 
      _reader(reader),
      _precision(precision),
      _readings(),
      _currentReadIndex(0),
      _runningTotal(0),
      _latestValue(-precision),
      _lastEmitTime(-throttleTimeMs),
      _enoughData(false)
    { };
    
    void loop() {
      _runningTotal -= _readings[_currentReadIndex];
      _readings[_currentReadIndex] = _reader.read();
      _runningTotal += _readings[_currentReadIndex];
      advanceReadIndex();
      
      if (!_enoughData) return;
      setLatestValue(runningAverage());
    }
    int calibratePrecision() {//todo: move to its own class?
      _precision = 1;
      long sumOfSquaredDifferences = 0;
      for (int i = 0; i < numReadings; ++i) {
        int difference = _readings[i] - runningAverage();
        sumOfSquaredDifferences += (long)difference * difference;
      }
      int deviation = sqrt(sumOfSquaredDifferences / numReadings);
      return 2* deviation;
    }
  };
  analogReader proximityRaw(A5);
  smoothInput proximityInput(proximityRaw, 40);

  void setup(){
    Serial.begin(9600);
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_YELLOW, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
  }

  void loop() {
    proximityInput.loop();
    
  }

