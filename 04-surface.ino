enum surface {
  SURFACE_NONE,
  SURFACE_RING, 
  SURFACE_EDGE
};
class surfaceInterpreter: public inputInterpreter<surface> {
  protected:
  virtual surface convert(int reflectance, int reflectanceRaw) {
    if (reflectance <= 750)
      return SURFACE_EDGE;
    return reflectance == reflectanceRaw && reflectanceRaw == 2500?
      SURFACE_NONE :
      SURFACE_RING;
  }
  virtual char* getName() {return "surface";}
};
