#ifndef _UDP_MESSAGE_MANAGEMENT_H
#define _UDP_MESSAGE_MANAGEMENT_H

// Arduino and ESP-IDF libraries that hold everything we need. 
#include <Arduino.h> 
#include <WiFi.h> 
#include <WiFiUdp.h> 
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <errno.h>

// Our own libraries 
#include <proto_msg_unpack.h>

// Constructor and destructor for our
// message management stuff!
void start_message_management(void);
void end_message_management_thread(void);

// Used so we know whether or not our callback initialization was accepted. 
enum MessageSubroutineSetupStatus{
    SUBROUTINE_ADD_SUCCESS, 
    SUBROUTINE_ADD_FAIL_MAX_NUM_REACHED, 
    SUBROUTINE_ADD_FAIL_UNKNOWN, 
    SUBROUTINE_REMOVE_SUCCESS, 
    SUBROUTINE_REMOVE_OUT_OF_BOUMDS,
    SUBROUTINE_REMOVE_FAIL_UNKNOWN
};

struct MessageSubroutineSetupReturn{
    MessageSubroutineSetupStatus setup_status; 
    uint32_t callback_handler_id; 
};

struct MessageReq{
    // Pointer to our message information
    uint8_t *data_ptr;
    // Pointer to our message length
    int data_len; 
};
extern MessageSubroutineSetupReturn add_subroutine_check(MessageData_MessageType msg_type, void (*func)(MessageReq *ptr));
#endif 
