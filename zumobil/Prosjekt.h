#ifndef Prosjekt_h
#define Prosjekt_h

#include "Arduino.h"
#include <PubSubClient.h>

  void connectWiFi(const char* ssid, const char* password);
  void sendWhatsAppMessage(String& message, String& phoneNumber, String& apiKey);
  void reconnectMQTT(PubSubClient& client);
  void wireTransmit(int espaddress, int totalTurns);

#endif