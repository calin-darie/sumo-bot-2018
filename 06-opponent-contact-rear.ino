class opponentContactRearInterpreter: public inputInterpreter<bool> {
  protected:
  virtual bool convert(int lightSmooth, int lightRaw) {
    return lightSmooth == 0;
  }
  virtual char* getName() {return "light";}
};