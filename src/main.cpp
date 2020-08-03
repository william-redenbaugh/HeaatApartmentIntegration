#include <Arduino.h>

// Our files 
#include "udp_message_management.h"
#include "wifi_setup.h"
#include "RGB_control.h"
#include "fan.h"


// RGB control and fan stuff. 
RGBControl led;
Fan fan;

void setup_hardware(void){
  Serial.begin(115200);
  // Setup the fan 
  fan.begin(GPIO_NUM_5, GPIO_NUM_16, GPIO_NUM_19);
  // Setup LED
  led.begin(GPIO_NUM_14, GPIO_NUM_22, GPIO_NUM_23);
  led.set_brightness(15);
  led.fade(20, 20, 0, 1000);
}

void led_control_callback(MessageReq *msg_req_ptr){
  // Getting and unpacking latest heaat message
  HeaatMessage heaat_message = unpack_heaat_message(msg_req_ptr->data_ptr, msg_req_ptr->data_len);
  led.fade(heaat_message.red, heaat_message.green, heaat_message.blue, 200);
  //Serial.println("message recieved!");
}

void setup() {
  // Setup LEDS and serial inferface ASAP
  setup_hardware();
  Serial.println();
  Serial.print("Connecting to wifi...");
  wifi_connect();
  Serial.println("connected!");

  Serial.print("Starting up message management thread...");
  start_message_management();
  Serial.println("complete!");

  // Setting up subroutine to deal with LED control
  add_subroutine_check(MessageData_MessageType_MATRIX_DATA, &led_control_callback);
}

void loop() {
  vTaskDelete(NULL);
}