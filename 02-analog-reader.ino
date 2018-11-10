class analogReader: public reader {
  const int _inputPin;
  public: 
  analogReader(int inputPin) : 
    _inputPin(inputPin){}

  int read() const{
    return analogRead(_inputPin);
  }    
};