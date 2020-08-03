#include "wifi_setup.h"

// General WiFi Information used for setup
const char network_name[] = "GoogleWatchesUsAll";
const char network_pass[] = "rapidpiano738";
const char udp_addr[] = "192.168.0.255";
bool connected = false; 

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          // When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.println(WiFi.localIP());  
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}

void wifi_connect(void){
  // delete old config
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  //register event handler
  WiFi.onEvent(WiFiEvent);
  //Initiate connection
  WiFi.begin(network_name, network_pass);
  Serial.println("Waiting for WIFI connection...");
  while(!connected){
      delay(100);
  }
}