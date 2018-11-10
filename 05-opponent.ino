//todo: add contact
class opponentVisibilityInterpreter: public inputInterpreter<bool> {
  protected:
  virtual bool convert(int proximity, int raw) {
    return proximity >= 150;
  }
};