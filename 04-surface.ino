enum surface {
  SURFACE_NONE,
  SURFACE_RING, 
  SURFACE_EDGE
};
class surfaceInterpreter: public inputInterpreter<surface> {
  protected:
  virtual surface convert(int reflectance) {
    if (reflectance <= 750)
      return SURFACE_EDGE;
    if (reflectance < 2500)
      return SURFACE_RING;
    return SURFACE_NONE;
  }
};
