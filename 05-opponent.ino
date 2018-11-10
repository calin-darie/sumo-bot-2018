//todo: add contact
class opponentVisibilityInterpreter: public inputInterpreter<bool> {
  protected:
  virtual bool convert(int proximity) {
    return proximity >= 150;
  }
};