/*
Author: William Redenbaugh
Last Edit Date: 2/1/2020
Notes:
Just a quick sketch to have an RGB light in our project
*/

#include "wifi_setup.h"
#include "mqtt_task.h"
#include "RGB_control.h"
#include "fan.h"
#include "WiFiUdp.h"
#include "WiFi.h"


#include "pb.h"
#include "pb_decode.h"
#include "pb_encode.h"

#include "messagedata.pb.h"
#include "heaat_message.pb.h"

// RGB control and fan stuff. 
RGBControl led;
Fan fan;

// Objects to deal with actual connection and mqtt stuff. 
WiFiClient client;

// Server to connect to via UDP. 
WiFiUDP udp; 

// Buffer holding all the juicy udp packet stuff.
uint8_t udp_packet_buffer[1024];

void setup_hardware(void){
  Serial.begin(115200);
  fan.begin(GPIO_NUM_5, GPIO_NUM_16, GPIO_NUM_19);
  
  led.begin(GPIO_NUM_14, GPIO_NUM_22, GPIO_NUM_23);
  led.set_brightness(15);
  led.fade(20, 20, 0, 1000);
}

void setup() {
  setup_hardware();
  setup_wifi();
  Serial.begin(9600);
  udp.begin(4040);
}

void loop() {
  int packet_size = udp.parsePacket();

  if(packet_size){
    int len = udp.read(udp_packet_buffer, 1024);
    if(len > 0){
      Serial.println("We received a message!");
      // We can get message data here!
      MessageData message_data; 
      {
        // Separating out the message data, and unpacking it!
        uint8_t message_proto[16];
        for(int i = 0; i < 16; i++)
          message_proto[i] = udp_packet_buffer[i];
        pb_istream_t msg_in = pb_istream_from_buffer(message_proto, 16);
        pb_decode(&msg_in, MessageData_fields, &message_data);
      }
      
      switch (message_data.message_type)
      {
      case(MessageData_MessageType_GENERAL_INSTRUCTIONS):
        break;
      case(MessageData_MessageType_HEAAT_CONTROL_DATA):{
        Serial.println("We received a heaat message!");
        HeaatMessage heaat_message; 
        // allocate space and unpack heaat message. 
        uint8_t msg_arr[message_data.message_size];
        for(uint16_t i = 0; i < message_data.message_size; i++)
          msg_arr[i] = udp_packet_buffer[16 + i];
        pb_istream_t msg_in = pb_istream_from_buffer(msg_arr, message_data.message_size);
        pb_decode(&msg_in, HeaatMessage_fields, &heaat_message);
        
        // Make sure value doesn't go out of bounds. 
        if (heaat_message.red > 255)
          heaat_message.red = 255; 
        if (heaat_message.red < 0)
          heaat_message.red = 0;
        if (heaat_message.green > 255)
          heaat_message.green = 255; 
        if (heaat_message.green < 0)
          heaat_message.green = 0; 
        if (heaat_message.blue > 255)
          heaat_message.blue = 255; 
        if (heaat_message.blue < 0)
          heaat_message.blue = 0; 
        
        // Send LED fade instruction
        led.fade(heaat_message.red, heaat_message.green, heaat_message.blue, 200);
      }

      // And we leave the scope out here!
        break;

      case(MessageData_MessageType_MATRIX_DATA):
        break;
      
      case(MessageData_MessageType_LED_STRIP_DATA):
        break;
      
      default:
        break;
      }
    }

  }
  
  // Just giving the rtos some free cpu cycles. 
  delay(100);
}
