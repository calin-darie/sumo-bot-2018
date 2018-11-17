enum surface {
  SURFACE_RING, 
  SURFACE_EDGE
};
class surfaceInterpreter: public inputInterpreter<surface> {
  private:
  static const int MAX_DECAY_TIME_MIN_REFLECTION = 2500;
  protected:
  virtual surface convert(int decayTime) {
    return decayTime == MAX_DECAY_TIME_MIN_REFLECTION ? 
      SURFACE_RING:
      SURFACE_EDGE;
  }
  public:
  virtual char* getName() {return "surface";}
};
