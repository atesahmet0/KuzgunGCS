#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include "mavlink/common/mavlink.h"

void setParameter(const std::string& param_name, float param_value,int sock , struct sockaddr_in& addr){
    mavlink_message_t msg;
    msg.msgid = MAVLINK_MSG_ID_PARAM_SET;
    mavlink_param_set_t param_set;
    param_set.target_system = 1;
    param_set.target_component = 1;
    std::strncpy(param_set.param_id, param_name.c_str(), sizeof(param_set.param_id));
    param_set.param_value = param_value;
    param_set.param_type = MAV_PARAM_TYPE_REAL32;

    mavlink_msg_param_set_encode(255, 0, &msg, &param_set);

    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    
    ssize_t bytes_sent = sendto(sock, buffer, len, 0,(struct sockaddr*)&addr, sizeof(addr));
    
    if (bytes_sent == -1) {
        std::cerr << "Mesaj gönderme hatası: " << strerror(errno) << std::endl;
    } else {
        std::cout << "Mesaj gönderildi (" << bytes_sent << " bytes)" << std::endl;
    }
}

