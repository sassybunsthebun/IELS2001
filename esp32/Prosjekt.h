#ifndef Prosjekt_h
#define Prosjekt_h

#include "Arduino.h"

class Prosjekt {
  public: 
    void connectWiFi(const char* ssid, const char* password);
    void sendMessages(String message, String phoneNumber, String apiKey);
    //void reconnectMQTT();
    void wireTransmit(byte zumoaddress, byte kjoremodus);
  private: 
    int _pin;
};

#endif