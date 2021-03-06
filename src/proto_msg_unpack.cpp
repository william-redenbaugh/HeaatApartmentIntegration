#include "proto_msg_unpack.h"

MessageData unpack_message_data(uint8_t *arr, uint32_t len);
TempuratureHumidityMessage unpack_temp_humd_data(uint8_t *arr, uint32_t len); 
RelayMessage unpack_relay_msg(uint8_t *arr, uint32_t len);

/* 
* @brief Allows us to read a pointer of information, and unpack that information. 
* @notes Be careful with pointers!
* params uint8_t *arr(pointer to array that contains the message)
* params uint32_t len(length of the array)
* returns MessageData struct with all the information
*/
MessageData unpack_message_data(uint8_t *arr, uint32_t len){
    MessageData msg_data;
    pb_istream_t msg_in = pb_istream_from_buffer(arr, len);
    pb_decode(&msg_in, MessageData_fields, &msg_data);
    return msg_data; 
}

/* 
* @brief Allows us to read a pointer of information, and unpack that information. 
* @notes Be careful with pointers!
* params uint8_t *arr(pointer to array that contains the message)
* params uint32_t len(length of the array)
* returns MessageData struct with all the information
*/
TempuratureHumidityMessage unpack_temp_humd_data(uint8_t *arr, uint32_t len){
    TempuratureHumidityMessage msg_data; 
    pb_istream_t msg_in = pb_istream_from_buffer(arr, len);
    pb_decode(&msg_in, TempuratureHumidityMessage_fields, &msg_data);
    return msg_data;
}

/* 
* @brief Allows us to read a pointer of information, and unpack that information. 
* @notes Be careful with pointers!
* params uint8_t *arr(pointer to array that contains the message)
* params uint32_t len(length of the array)
* returns MessageData struct with all the information
*/
RelayMessage unpack_relay_msg(uint8_t *arr, uint32_t len){
    RelayMessage msg_data; 
    pb_istream_t msg_in = pb_istream_from_buffer(arr, len);
    pb_decode(&msg_in, RelayMessage_fields, &msg_data);
    return msg_data; 
}

/* 
* @brief Allows us to read a pointer of information, and unpack that information. 
* @notes Be careful with pointers!
* params uint8_t *arr(pointer to array that contains the message)
* params uint32_t len(length of the array)
* returns HeaatMessage struct with all the information
*/
HeaatMessage unpack_heaat_message(uint8_t *arr, uint32_t len){
    HeaatMessage msg_data; 
    pb_istream_t msg_in = pb_istream_from_buffer(arr, len);
    pb_decode(&msg_in, HeaatMessage_fields, &msg_data);

    // Just in case we get info that's out of bounds 
    if(msg_data.red > 255)
        msg_data.red = 255;
    if(msg_data.green > 255)
        msg_data.green = 255; 
    if(msg_data.blue > 255)
        msg_data.blue = 255; 
    return msg_data; 
}