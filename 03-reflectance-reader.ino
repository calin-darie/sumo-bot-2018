#include <QTRSensors.h>

class reflectancePairReader {
  private: 
  static const int NUM_REFLECTANCE_SENSORS = 2;
  static const int TIMEOUT = 2500;
  QTRSensorsRC _qtrrc;
  unsigned int _sensorValues[NUM_REFLECTANCE_SENSORS];
  bool _sensorStale[NUM_REFLECTANCE_SENSORS];

  public:
  reflectancePairReader(unsigned char pins[NUM_REFLECTANCE_SENSORS]) :
    _qtrrc(pins, NUM_REFLECTANCE_SENSORS, TIMEOUT)
  {  
    for (int i = 0; i< NUM_REFLECTANCE_SENSORS; ++i) {
      _sensorStale[i] = true;
    }
  }
  int read(int index) { 
    if (_sensorStale[index]) {
      _qtrrc.read(_sensorValues);
      memset(_sensorStale, 0, sizeof(_sensorStale));
    }
    _sensorStale[index] = true;
    return _sensorValues[index];
  }
};

class reflectanceReader: public reader {
  private: 
  int _sensorIndex;
  reflectancePairReader& _pairReader;
  
  public: 
  reflectanceReader(
    reflectancePairReader& pairReader,
    int sensorIndex): 
    _sensorIndex(sensorIndex),
    _pairReader(pairReader)
  { }
  
  int read() const{
    return _pairReader.read(_sensorIndex);
  }
};