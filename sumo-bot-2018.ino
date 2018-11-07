#define PIN_YELLOW 11
#define PIN_RED 12
#define PIN_GREEN 13

#define IN_LIGHT_REARLEFT 0
#define IN_LIGHT_REARRIGHT 4

class smoothInput {
  private:
  static const int numReadings = 10;

  int inputPin;
  int readings[numReadings];
  int currentReadIndex;
  int runningTotal;
  int precision;
  int latestValue;
  bool enoughData;
  
  static int roundToPrecision(int value, int precision) {
    return (value + precision / 2) / precision * precision;
  }
  
  inline int runningAverage() {
    return runningTotal / numReadings;
  }
  
  void advanceReadIndex() {
    ++ currentReadIndex;
    if (currentReadIndex  >= numReadings ) {
      currentReadIndex = 0;
      enoughData = true;
    }
  }
  
  void setLatestValue(int value) {
    if (abs(latestValue - value) < precision) return;
    //onValueChanged(latestValue, value);
    latestValue = value;
    Serial.println(roundToPrecision(value, precision));
  }
  
  public: 
  smoothInput(int inputPin, int precision)
    : readings(),
      currentReadIndex(0),
      runningTotal(0),
      inputPin(inputPin),
      precision(precision),
      latestValue(-1),
      enoughData(false)
  { };
  
  void loop() {
    runningTotal -= readings[currentReadIndex];
    readings[currentReadIndex] = analogRead(inputPin);
    runningTotal += readings[currentReadIndex];
    advanceReadIndex();
    
    if (!enoughData) return;
    setLatestValue(runningAverage());
  }
  int calibratePrecision() {
    precision = 1;
    long sumOfSquaredDifferences = 0;
    for (int i = 0; i < numReadings; ++i) {
      int difference = readings[i] - runningAverage();
      sumOfSquaredDifferences += (long)difference * difference;
    }
    int deviation = sqrt(sumOfSquaredDifferences / numReadings);
    return 2* deviation;
  }
};
smoothInput proximity(A5, 40);

void setup(){
  Serial.begin(9600);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
}

void loop() {
  proximity.loop();
  
}
