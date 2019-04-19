

class Channel {
  public:
  int channel;         // which channel should be used for IO

  Channel(int chan) {
    this->channel = chan;
  }
  void clearEvents();
};


// CLEAR ALL EVENTS
void Channel::clearEvents() {

}

// QUANTIZE EVENTS
