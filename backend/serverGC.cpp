#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "mavlink/common/mavlink.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <map>
#include <set>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include "mission.hpp"
#include <sstream>
#include <string>
using namespace std;

#define UDP_PORT 14550
bool received_fly_command = false;
typedef websocketpp::server<websocketpp::config::asio> WebSocketServer;
typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> ConnectionList;

// WebSocket sunucusu ve bağlantı listesi
WebSocketServer server;
ConnectionList connections;

// Son alınan veriler
mavlink_gps_raw_int_t latest_gps_raw;
mavlink_raw_imu_t latest_imu_data;
mavlink_heartbeat_t latest_heartbeat;
mavlink_rc_channels_raw_t latest_rc_channels;
mavlink_battery_status_t latest_battery_status;

// GPS verilerini JSON formatında dönüştürme
std::string gps_data_to_json(const mavlink_gps_raw_int_t &gps_raw) {
    std::stringstream json_data;
    json_data << "{"
              << "\"latitude\": " << gps_raw.lat / 1e7 << ", "
              << "\"longitude\": " << gps_raw.lon / 1e7 << ", "
              << "\"altitude\": " << gps_raw.alt / 1000 << ", "
              << "\"fix_type\": " << (int)gps_raw.fix_type << ", "
              << "\"satellites_visible\": " << (int)gps_raw.satellites_visible
              << "}";
    return json_data.str();
}

// IMU verilerini JSON formatında dönüştürme
std::string imu_data_to_json(const mavlink_raw_imu_t &imu_data) {
    std::stringstream json_data;
    json_data << "{"
              << "\"accel_x\": " << imu_data.xacc << ", "
              << "\"accel_y\": " << imu_data.yacc << ", "
              << "\"accel_z\": " << imu_data.zacc << ", "
              << "\"gyro_x\": " << imu_data.xgyro << ", "
              << "\"gyro_y\": " << imu_data.ygyro << ", "
              << "\"gyro_z\": " << imu_data.zgyro << ", "
              << "\"mag_x\": " << imu_data.xmag << ", "
              << "\"mag_y\": " << imu_data.ymag << ", "
              << "\"mag_z\": " << imu_data.zmag
              << "}";
    return json_data.str();
}

// Heartbeat verilerini JSON formatında dönüştürme
std::string heartbeat_to_json(const mavlink_heartbeat_t &heartbeat) {
    std::stringstream json_data;
    json_data << "{"
              << "\"type\": " << (int)heartbeat.type << ", "
              << "\"autopilot\": " << (int)heartbeat.autopilot << ", "
              << "\"base_mode\": " << (int)heartbeat.base_mode << ", "
              << "\"system_status\": " << (int)heartbeat.system_status
              << "}";
    return json_data.str();
}

// RC verilerini JSON formatında dönüştürme
std::string rc_channels_data_to_json(const mavlink_rc_channels_raw_t &rc_channels) {
    std::stringstream json_data;
    json_data << "{"
              << "\"chan1_raw\": " << rc_channels.chan1_raw << ", "
              << "\"chan2_raw\": " << rc_channels.chan2_raw << ", "
              << "\"chan3_raw\": " << rc_channels.chan3_raw << ", "
              << "\"chan4_raw\": " << rc_channels.chan4_raw << ", "
              << "\"chan5_raw\": " << rc_channels.chan5_raw << ", "
              << "\"chan6_raw\": " << rc_channels.chan6_raw << ", "
              << "\"chan7_raw\": " << rc_channels.chan7_raw << ", "
              << "\"chan8_raw\": " << rc_channels.chan8_raw
              << "}";
    return json_data.str();
}

// Batarya verilerini JSON formatında dönüştürme
std::string battery_data_to_json(const mavlink_battery_status_t &battery_status) {
    std::stringstream json_data;
    json_data << "{"
              << "\"voltage\": " << battery_status.voltages[0] / 1000.0 << ", "
              << "\"current\": " << battery_status.current_battery / 100.0 << ", "
              << "\"remaining\": " << (int)battery_status.battery_remaining
              << "}";
    return json_data.str();
}

// Tüm verileri JSON formatında birleştirme
std::string all_data_to_json() {
    std::stringstream json_data;
    json_data << "{"
              << "\"gps\": " << gps_data_to_json(latest_gps_raw) << ", "
              << "\"imu\": " << imu_data_to_json(latest_imu_data) << ", "
              << "\"heartbeat\": " << heartbeat_to_json(latest_heartbeat) << ", "
              << "\"rc_channels\": " << rc_channels_data_to_json(latest_rc_channels) << ", "
              << "\"battery\": " << battery_data_to_json(latest_battery_status)
              << "}";
    return json_data.str();
}

// WebSocket üzerinden veri gönderme
void broadcast_data(const std::string &data) {
    for (auto &hdl : connections) {
        try {
            server.send(hdl, data, websocketpp::frame::opcode::text);
        } catch (const websocketpp::exception &e) {
            std::cerr << "Error sending message: " << e.what() << std::endl;
        }
    }
}

// MAVLink mesajlarını işleme
void process_mavlink_message(mavlink_message_t &msg) {
    switch (msg.msgid) {
        case MAVLINK_MSG_ID_GPS_RAW_INT: {
            mavlink_msg_gps_raw_int_decode(&msg, &latest_gps_raw);
            break;
        }
        case MAVLINK_MSG_ID_RAW_IMU: {
            mavlink_msg_raw_imu_decode(&msg, &latest_imu_data);
            break;
        }
        case MAVLINK_MSG_ID_HEARTBEAT: {
            mavlink_msg_heartbeat_decode(&msg, &latest_heartbeat);
            break;
        }
        case MAVLINK_MSG_ID_RC_CHANNELS_RAW: {
            mavlink_msg_rc_channels_raw_decode(&msg, &latest_rc_channels);
            break;
        }
        case MAVLINK_MSG_ID_BATTERY_STATUS: {
            mavlink_msg_battery_status_decode(&msg, &latest_battery_status);
            break;
        }
        default:
            uint8_t system_id = msg.sysid;
            uint8_t component_id = msg.compid;

            std::cout << "Received message from system_id=" << (int)system_id
              << ", component_id=" << (int)component_id << std::endl;

            std::cout << "Unknown message ID: " << msg.msgid << std::endl;
            break;
    }
}

void fly_command(int sock,sockaddr_in addr){
    
    // Önceki mission'ı temizle
    send_mission_clear_all(sock,addr);
    sleep(2);

    // Mission count gönder
    std::cout << "Sending mission count..." << std::endl;
    send_mission_count(sock, addr, mission_points.size());
    sleep(2);

    // Mission itemları gönder
    for (size_t i = 0; i < mission_points.size(); i++) {
        std::cout << "Sending mission item " << i << std::endl;
        send_mission_item(sock, addr, i, mission_points[i]);
        usleep(100000);
    }
    sleep(2);

    // Auto mode'a geç
    std::cout << "Setting AUTO mode..." << std::endl;
    send_set_mode_command(sock, addr, PX4_CUSTOM_MAIN_MODE_AUTO);
    sleep(2);
    // Arm et
    std::cout << "Arming drone..." << std::endl;
    send_arm_command(sock, addr, true);
    sleep(3);

    // Mission'ı başlat
    std::cout << "Starting mission..." << std::endl;
    send_mission_start_command(sock, addr);

    // Heartbeat'i devam ettir
    std::cout << "Continuing heartbeat..." << std::endl;

}

int main() {
    //
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation failed" << std::endl;
    }

    // Adres yapılandırması
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(14580);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //

    // WebSocket sunucusu ayarları
    server.clear_access_channels(websocketpp::log::alevel::all);
    server.set_access_channels(websocketpp::log::alevel::connect);
    server.set_access_channels(websocketpp::log::alevel::disconnect);
    server.set_access_channels(websocketpp::log::alevel::app);

    server.init_asio();
    server.set_reuse_addr(true);

    // WebSocket event handlers
    server.set_open_handler([](websocketpp::connection_hdl hdl) {
        connections.insert(hdl);
        std::cout << "New WebSocket connection" << std::endl;
    });

    server.set_close_handler([](websocketpp::connection_hdl hdl) {
        connections.erase(hdl);
        std::cout << "WebSocket connection closed" << std::endl;
    });

    // UDP soketi oluştur
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        std::cerr << "Error creating UDP socket" << std::endl;
        return -1;
    }

    // UDP adres yapılandırması
    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    udp_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    // UDP soketini bağla
    if (bind(udp_sockfd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        std::cerr << "Error binding UDP socket" << std::endl;
        close(udp_sockfd);
        return -1;
    }

    // WebSocket sunucusunu başlat
    server.listen(4444);
    server.start_accept();
    
    // WebSocket sunucusu için thread
    std::thread ws_thread([&]() {
        server.run();
    });

    server.set_message_handler([](websocketpp::connection_hdl hdl, WebSocketServer::message_ptr msg) {
        std::string payload = msg->get_payload();

        stringstream ss(payload);
        std::string item; 
        if (payload == "fly") {
            received_fly_command = true;
        }
        if(payload.find("waypoint") != std::string::npos){
            std::vector<std::string> result;
            std::stringstream ss(payload);
            std::string item;

            while (getline(ss, item, ',')) {
                result.push_back(item);
            }
            MissionPoint ms = {stod(result[1]),stod(result[2])};
            mission_points.push_back(ms);
            
        }
    });
    // MAVLink mesajı ve buffer
    mavlink_message_t msg;
    unsigned char mavlink_buffer[1024];

    while (true) {
        // UDP üzerinden veri al
        socklen_t addr_len = sizeof(udp_addr);
        int len = recvfrom(udp_sockfd, mavlink_buffer, sizeof(mavlink_buffer), 0,
                          (struct sockaddr *)&udp_addr, &addr_len);

        if (len > 0) {
            // Gelen veriyi MAVLink mesajına dönüştür
            for (int i = 0; i < len; i++) {
                if (mavlink_parse_char(MAVLINK_COMM_0, mavlink_buffer[i], &msg, NULL)) {
                    // Mesajı işle ve WebSocket üzerinden gönder
                    process_mavlink_message(msg);
                    std::string json = all_data_to_json();
                    broadcast_data(json);
                }
            }
        }
        if (received_fly_command) {
            fly_command(sock,addr);
            received_fly_command = false; // Flag'i sıfırla
        }
        send_heartbeat(sock,addr);
        usleep(10000);  // 10ms bekle
    }

    // Temizlik
    close(udp_sockfd);
    server.stop();
    ws_thread.join();

    return 0;
}