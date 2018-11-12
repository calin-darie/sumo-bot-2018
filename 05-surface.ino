enum surface {
  SURFACE_NONE,
  SURFACE_RING, 
  SURFACE_EDGE
};
class surfaceInterpreter: public inputInterpreter<surface> {
  private:
  bool _rawSaidNone;
  static const int MAX_DECAY_TIME_MIN_REFLECTION = 2500;
  protected:
  virtual surface convertLatestRawAverage(int decayTime) {
    _rawSaidNone = decayTime == MAX_DECAY_TIME_MIN_REFLECTION;
    return _rawSaidNone? 
      SURFACE_NONE :
      getLatest();
  }
  virtual surface convert(int decayTime) {
    if (decayTime <= 750)
      return SURFACE_EDGE;
    return decayTime == MAX_DECAY_TIME_MIN_REFLECTION && _rawSaidNone?
      SURFACE_NONE :
      SURFACE_RING;
  }
  public:
  virtual char* getName() {return "surface";}
};
