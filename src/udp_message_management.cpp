#include "udp_message_management.h"
#include <list> 

// Constructor and destructor for our
// message management stuff!
void start_message_management(void);
void end_message_management_thread(void);
void create_udp_server(void);
void udp_management_thread(void *parameters); 
void check_new_data(void);
void check_req_res(void);
void message_management_req_thread(void *parameters);
extern MessageSubroutineSetupReturn add_subroutine_check(MessageData_MessageType msg_type, void (*func)(MessageReq *ptr));

#define SERVER_PORT 4040

// To prevent ourselves from having too many local variables, we put them all 
// In a single struct
struct {
    TaskHandle_t udp_server_task_handle;           // Handler for our udp reading task. 
    TaskHandle_t message_req_task_handle;
    int socket_handle = -1;             // Handler ID for our UDP socket 
    uint8_t rx_buff[7000];              // Buffer of information that we store our socket info into. 
    MessageData latest_message_data;    // Place where we can hold our latest message data. 
    int latest_message_length = 0; 
}msg_manage;

/*
* @brief: Struct that helps us deal with message subroutine callbackets
* @notes: Switched from linkedlist to predefined array for preformance reasons. 
*/
struct MessageSubroutine{
    // Pass in function as arguement
    void (*func)(MessageReq *ptr);
    // Which type of message we are looking for
    MessageData_MessageType msg_type;
};
#define MAX_MESSAGE_SUBROUTINE_NUM 48
MessageSubroutine message_subroutine_list[MAX_MESSAGE_SUBROUTINE_NUM]; 
volatile int num_subroutines = 0; 

/* 
* @brief General call that will let us deal with all of our message management stuff
* @notes Should generally be called at begining of runtime, before other subsytems are generated. 
* params none
* returns none
*/
void start_message_management(void){
    
    create_udp_server();

    // Start up UDP Management thread!
    xTaskCreatePinnedToCore(
      udp_management_thread,                    /* Function to implement the task */
      "Message Management Thread",              /* Name of the task */
      10000,                                    /* Stack size in words */
      NULL,                                     /* Task input parameter */
      32,                                       /* Priority of the task */
      &msg_manage.udp_server_task_handle,       /* Task handle. */
      1);                                       /* Core where the task should run */
}


/* 
* @brief General call that will let us deal with all of our message management stuff
* @notes Should generally be called at begining of runtime, before other subsytems are generated. 
* params none
* returns none
*/
void create_udp_server(void){
    // Generate our socket
    if((msg_manage.socket_handle = socket(AF_INET, SOCK_DGRAM, 0)) == -1){
        log_e("Socket couldn't be generated");
    }

    int yes = 1; 
    // Enable certain socket options
    if(setsockopt(msg_manage.socket_handle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0){
        log_e("Couldn't set the socket option %d", errno);
        close(msg_manage.socket_handle);
    }

    // Set oursleves to any particular IP address!
    IPAddress address = IPAddress(INADDR_ANY); 
    // Createing our input socket stuff
    struct sockaddr_in addr;
    memset((char *) &addr, 0, sizeof(addr));
    
    // Putting all of our port and address information into the right spot 
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = (in_addr_t)address;
    
    // Bind our socket should everything work as expected. 
    if(bind(msg_manage.socket_handle , (struct sockaddr*)&addr, sizeof(addr)) == -1){
        log_e("could not bind socket: %d", errno);
        close(msg_manage.socket_handle);
        return;
    }
    // Manipulates our file descriptor of our UDP socket. 
    fcntl(msg_manage.socket_handle, F_SETFL, O_NONBLOCK);
}

/* 
* @brief General call that will let us deal with all of our message management stuff
* @notes Should generally be called at begining of runtime, before other subsytems are generated. 
* params none
* returns none
*/
void end_message_management(void){
    close(msg_manage.socket_handle);
    // Deletes the UDP server. 
    vTaskDelete(msg_manage.udp_server_task_handle);
}
    
/* 
* @brief Thread space that we will use to deal with all of our udp management stuff in 
* @notes Don't call all willy nilly since there is a loop in there, and the thread isn't suppose to stop
* params void *parameters pointer to struct of information
* returns none
*/
void udp_management_thread(void *parameters){
    TickType_t start_loop; 

    for(;;){
        start_loop = xTaskGetTickCount();
        check_new_data();
        // Let program run every 70 milliseconds
        vTaskDelayUntil(&start_loop, 100/portTICK_PERIOD_MS);
    }   
}

/* 
* @brief Whenever we are checking for new data, it's placed here 
* @notes Called in the thread. 
* params none
* returns none
*/
void check_new_data(void){
    struct sockaddr_in rev_addr; 
    int slen = 0; 
    msg_manage.latest_message_length = recvfrom( msg_manage.socket_handle, // Handler that contains our Socket ID
                        msg_manage.rx_buff,                                // Buffer that we will put our socket information into
                        sizeof(msg_manage.rx_buff),                        // Size of the buffer that we are giving to the socket
                        MSG_DONTWAIT,                                      // Don't block until message has received, just keep through
                        (struct sockaddr *) &rev_addr,                     // Place where we can save the address that sends us our information
                        (socklen_t *)&slen);                               // Length of the socket packet. 
    // Minimum message size is the messagedata header packet. 
    if(msg_manage.latest_message_length >= 16){
        // Notice that we only told it to look at the first 16 bits of information
        // That's because the header will always be 16 bytes no matter what. 
        msg_manage.latest_message_data = unpack_message_data(msg_manage.rx_buff, 16);  
        // Choose which request response we have
        check_req_res();
    }
}

/* 
* @brief Checks which of our request->response systems need to be interrupted 
* @notes Should only be called from within the udp_message_management file
* params none
* returns none
*/
void check_req_res(void){
    // Going through all of our subroutine calls and run their res-req stuff. 
    for(uint32_t n = 0; n < num_subroutines; n++){
            // If we have the latest message, then 
            if(message_subroutine_list[n].msg_type == msg_manage.latest_message_data.message_type){
            // Generate the message, of which we will set a pointer to information
                MessageReq msg_reg = {
                // Moving pointer up 16 spots, since we don't need the message
                // Header anymore. 
                (msg_manage.rx_buff + 16), 
                // How long the message is minus the 
                (msg_manage.latest_message_length - 16)
            };
            message_subroutine_list[n].func(&msg_reg);
        }
    }
}

/* 
* @brief Easy method for dealing with new messages coming into the system 
* @notes Just makes callbacks easy to deal with, you still need to deal with the deserialization
* And unpacking yourself. 
* params MessageData_MessageType which type of message data are we sending?
* params void(*func)(MessageReq *ptr) callback for dealing with subroutines. 
* returns MessageSubroutineSetupStatus whether or not we actually go the information we needed. 
*/
extern MessageSubroutineSetupReturn add_subroutine_check(MessageData_MessageType msg_type, void(*func)(MessageReq *ptr)){
    if(num_subroutines > MAX_MESSAGE_SUBROUTINE_NUM){
        MessageSubroutineSetupReturn err_msg = {SUBROUTINE_ADD_FAIL_MAX_NUM_REACHED, 0};
        return err_msg;
    }
    else{
        message_subroutine_list[num_subroutines].func = func;
        message_subroutine_list[num_subroutines].msg_type = msg_type;
        MessageSubroutineSetupReturn success_msg = {SUBROUTINE_ADD_FAIL_MAX_NUM_REACHED, (uint32_t)num_subroutines};
        num_subroutines++; 
        return success_msg; 
    }
}

/* 
* @brief Easy method for removing a callback from the list. 
* @notes Just makes callbacks easy to deal with, you still need to deal with the deserialization
* And unpacking yourself.  Also this is super slow, so try not to do it too often. 
* params uint32_t id of which message callback we want to remove.  
* returns MessageSubroutineSetupStatus whether or not we actually go the information we needed. 
*/
extern MessageSubroutineSetupStatus remove_message_callback(uint32_t id){
    if(id < num_subroutines){
        // Moving all the values down the array. The hope is this function won't be called too often, since this takes O(n) time
        for(id = id; id < num_subroutines-1; id++)
            message_subroutine_list[id] = message_subroutine_list[id+1];
        
        // Clear out last value in the array 
        memset(&message_subroutine_list[num_subroutines-1], 0, sizeof(MessageSubroutine));
        num_subroutines--; 
        return SUBROUTINE_REMOVE_SUCCESS;
    }   
    // If we are trying to reach a subroutine that hasn't been touched yet. 
    else 
        return SUBROUTINE_REMOVE_OUT_OF_BOUMDS;
}