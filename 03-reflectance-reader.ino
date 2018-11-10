#include <QTRSensors.h>

class reflectanceReader: public reader {
  private: 
  static const int NUM_SENSORS = 2;
  static const int TIMEOUT = 2500;
  static QTRSensorsRC qtrrc;
  static unsigned int sensorValues[NUM_SENSORS];
  int _sensorIndex;
  
  public: 
  reflectanceReader(int sensorIndex): 
    _sensorIndex(sensorIndex)
  { }
  
  int read() const{ 
    qtrrc.read(sensorValues);
    return sensorValues[_sensorIndex];
  }
};
QTRSensorsRC reflectanceReader::qtrrc((unsigned char[]) { 7, 8 }, NUM_SENSORS, TIMEOUT);
unsigned int reflectanceReader::sensorValues[NUM_SENSORS];