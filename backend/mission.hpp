#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "mavlink/common/mavlink.h"
#include <cmath>

// MAVLink komut enumları
enum MAV_CMD {
    MAV_CMD_NAV_WAYPOINT = 16,
    MAV_CMD_NAV_LOITER_UNLIM = 17,
    MAV_CMD_NAV_LOITER_TURNS = 18,
    MAV_CMD_NAV_LOITER_TIME = 19,
    MAV_CMD_NAV_RETURN_TO_LAUNCH = 20,
    MAV_CMD_NAV_LAND = 21,
    MAV_CMD_NAV_TAKEOFF = 22,
    MAV_CMD_COMPONENT_ARM_DISARM = 400,
    MAV_CMD_DO_SET_MODE = 176,
    MAV_CMD_MISSION_START = 300
};

// Set mode için gerekli enum
enum PX4_CUSTOM_MAIN_MODE {
    PX4_CUSTOM_MAIN_MODE_MANUAL = 1,
    PX4_CUSTOM_MAIN_MODE_ALTCTL = 2,
    PX4_CUSTOM_MAIN_MODE_POSCTL = 3,
    PX4_CUSTOM_MAIN_MODE_AUTO = 4,
    PX4_CUSTOM_MAIN_MODE_ACRO = 5,
    PX4_CUSTOM_MAIN_MODE_OFFBOARD = 6,
    PX4_CUSTOM_MAIN_MODE_STABILIZED = 7,
    PX4_CUSTOM_MAIN_MODE_RATTITUDE = 8
};

struct MissionPoint {
    double lat;
    double lon;
    float alt;
};


std::vector<MissionPoint> mission_points = {
};

// Heartbeat mesajı gönderme
void send_heartbeat(int sock, const struct sockaddr_in& addr) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    mavlink_msg_heartbeat_pack(255, 0, &msg, MAV_TYPE_GCS, MAV_AUTOPILOT_INVALID, 0, 0, 0);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));
}

// Set mode komutu gönderme
void send_set_mode_command(int sock, const struct sockaddr_in& addr, uint8_t mode) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    mavlink_command_long_t cmd = {0};
    cmd.target_system = 1;
    cmd.target_component = 1;
    cmd.command = MAV_CMD_DO_SET_MODE;
    cmd.param1 = mode;
    cmd.param2 = 0;
    cmd.param3 = 0;

    mavlink_msg_command_long_encode(1, 0, &msg, &cmd);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    std::cout << "Set mode command sent: " << (int)mode << std::endl;
}

// Arm komutu gönderme
void send_arm_command(int sock, const struct sockaddr_in& addr, bool arm) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    mavlink_command_long_t cmd = {0};
    cmd.target_system = 1;
    cmd.target_component = 1;
    cmd.command = MAV_CMD_COMPONENT_ARM_DISARM;
    cmd.confirmation = 0;
    cmd.param1 = arm ? 1.0f : 0.0f;
    cmd.param2 = 21196.0f;  // Force arm/disarm

    mavlink_msg_command_long_encode(1, 0, &msg, &cmd);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    std::cout << (arm ? "Arm" : "Disarm") << " command sent" << std::endl;
}

// Mission count mesajı gönderme
void send_mission_count(int sock, const struct sockaddr_in& addr, uint16_t count) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    mavlink_msg_mission_count_pack(255, 0, &msg, 1, 1, count,0,0);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    std::cout << "Mission count sent: " << count << std::endl;
}

// Mission item gönderme
void send_mission_item(int sock, const struct sockaddr_in& addr, uint16_t seq, const MissionPoint& point) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    mavlink_mission_item_int_t mission_item = {0};
    mission_item.target_system = 1;
    mission_item.target_component = 1;
    mission_item.seq = seq;
    mission_item.frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
    mission_item.command = MAV_CMD_NAV_WAYPOINT;
    mission_item.current = (seq == 0) ? 1 : 0;
    mission_item.autocontinue = 1;
    mission_item.param1 = 0;  // Hold time
    mission_item.param2 = 2;  // Acceptance radius
    mission_item.param3 = 0;  // Pass through
    mission_item.param4 = NAN;  // Yaw
    mission_item.x = (int32_t)(point.lat * 1e7);
    mission_item.y = (int32_t)(point.lon * 1e7);
    mission_item.z = 50.0;

    mavlink_msg_mission_item_int_encode(255, 0, &msg, &mission_item);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));

    std::cout << "Sent mission item " << seq << ": Lat=" << point.lat
              << " Lon=" << point.lon << " Alt=" << point.alt << std::endl;
}

// Mission başlatma komutu
void send_mission_start_command(int sock, const struct sockaddr_in& addr) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    mavlink_command_long_t cmd = {0};
    cmd.target_system = 1;
    cmd.target_component = 1;
    cmd.command = MAV_CMD_MISSION_START;
    cmd.confirmation = 0;

    mavlink_msg_command_long_encode(255, 0, &msg, &cmd);
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    
    std::cout << "Mission start command sent" << std::endl;
}

void wait_for_mission_ack(int sock) {
    mavlink_message_t msg;
    mavlink_mission_ack_t ack;

    while (true) {
        // Mesajı al
        ssize_t len = recv(sock, &msg, sizeof(msg), 0);
        if (len > 0 && msg.msgid == MAVLINK_MSG_ID_MISSION_ACK) {
            // MISSION_ACK mesajını çözümle
            mavlink_msg_mission_ack_decode(&msg, &ack);

            // Gelen sonucu kontrol et
            if (ack.type == MAV_MISSION_ACCEPTED) {
                std::cout << "Mission accepted by the vehicle." << std::endl;
                break;
            } else {
                std::cerr << "Mission rejected! Reason: " << (int)ack.type << std::endl;
                break;
            }
        }
    }
}

void send_mission_clear_all(int sock, const struct sockaddr_in& addr) {
    mavlink_message_t msg;
    uint8_t buffer[300];

    // MAV_CMD_MISSION_CLEAR_ALL komutunu oluştur
    mavlink_command_long_t cmd = {0};
    cmd.target_system = 1;  // Hedef sistem (1 genellikle PX4'ün system_id'sidir)
    cmd.target_component = 1;  // Hedef bileşen (1 genellikle PX4'ün component_id'sidir)
    cmd.command = 41;  // Komut: Tüm görevleri sil
    cmd.confirmation = 0;  // Onay isteme

    // Komutu MAVLink mesajına kodla
    mavlink_msg_command_long_encode(1, 0, &msg, &cmd);

    // Mesajı gönder
    uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
    sendto(sock, buffer, len, 0, (struct sockaddr*)&addr, sizeof(addr));

    std::cout << "Mission clear all command sent" << std::endl;
}