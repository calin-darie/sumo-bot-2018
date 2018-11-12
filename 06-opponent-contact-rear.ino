class opponentContactRearInterpreter: public inputInterpreter<bool> {
  protected:
  virtual bool convert(int light) {
    return light == 0;
  }
  virtual char* getName() {return "light";}
};