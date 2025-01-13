#include <iostream>
#include <vector>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "mavlink/common/mavlink.h"
#include <cmath>
#include <sstream>

using namespace std;

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>



std::string missionItemToJson(const mavlink_mission_item_int_t& item) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(7);
    
    ss << "{";
    ss << "\"seq\":" << item.seq << ",";
    ss << "\"frame\":" << static_cast<int>(item.frame) << ",";
    ss << "\"command\":" << item.command << ",";
    ss << "\"current\":" << static_cast<int>(item.current) << ",";
    ss << "\"autocontinue\": " << static_cast<int>(item.autocontinue) << ",";
    ss << "\"param1\":" << item.param1 << ",";
    ss << "\"param2\":" << item.param2 << ",";
    ss << "\"param3\":" << item.param3 << ",";
    ss << "\"param4\":" << item.param4 << ",";
    ss << "\"x\":" << (item.x / 1e7) << ","; 
    ss << "\"y\":" << (item.y / 1e7) << ","; 
    ss << "\"z\":" << item.z << ",";
    ss << "\"mission_type\": " << static_cast<int>(item.mission_type) << "";
    ss << "}";
    
    return ss.str();
}

std::string getCommandString(uint16_t command) {
    switch(command) {
        case MAV_CMD_NAV_WAYPOINT: return "WAYPOINT";
        case MAV_CMD_NAV_LOITER_UNLIM: return "LOITER_UNLIMITED";
        case MAV_CMD_NAV_LOITER_TURNS: return "LOITER_TURNS";
        case MAV_CMD_NAV_LOITER_TIME: return "LOITER_TIME";
        case MAV_CMD_NAV_RETURN_TO_LAUNCH: return "RETURN_TO_LAUNCH";
        case MAV_CMD_NAV_LAND: return "LAND";
        case MAV_CMD_NAV_TAKEOFF: return "TAKEOFF";
        default: return "UNKNOWN_COMMAND";
    }
}

std::string missionItemToHumanReadable(const mavlink_mission_item_int_t& item) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(7);
    
    ss << "Görev #" << item.seq << ":\n";
    ss << "  Komut: " << getCommandString(item.command) << " (" << item.command << ")\n";
    ss << "  Koordinatlar: " << (item.x / 1e7) << ", " << (item.y / 1e7) << ", " << item.z << "m\n";
    
    switch(item.command) {
        case MAV_CMD_NAV_WAYPOINT:
            ss << "  Bekleme Süresi: " << item.param1 << " saniye\n";
            ss << "  Kabul Yarıçapı: " << item.param2 << " metre\n";
            break;
        case MAV_CMD_NAV_LOITER_TIME:
            ss << "  Loiter Süresi: " << item.param1 << " saniye\n";
            break;
        case MAV_CMD_NAV_TAKEOFF:
            ss << "  Kalkış Açısı: " << item.param1 << " derece\n";
            break;
    }
    
    return ss.str();
}


class UDPMissionDownloader {
private:
    std::vector<mavlink_mission_item_int_t> mission_items;
    const int TIMEOUT_MS = 500; 
    uint16_t expected_seq = 0;
    uint8_t system_id;
    uint8_t component_id;
    int retry_count = 0;
    const int MAX_RETRIES = 5;

    int sock;
    struct sockaddr_in gcAddr;
    struct sockaddr_in locAddr;
    uint8_t buf[MAVLINK_MAX_PACKET_LEN];
    
public:
    UDPMissionDownloader(const char* target_ip = "127.0.0.1", 
                        int target_port = 14550,
                        int local_port = 14555,
                        uint8_t sysid = 1, 
                        uint8_t compid = 1) 
        : system_id(sysid), component_id(compid) {
        
        sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == -1) {
            throw std::runtime_error("UDP socket oluşturulamadı");
        }

        memset(&locAddr, 0, sizeof(locAddr));
        locAddr.sin_family = AF_INET;
        locAddr.sin_addr.s_addr = INADDR_ANY;
        locAddr.sin_port = htons(local_port);

        if (bind(sock, (struct sockaddr *)&locAddr, sizeof(struct sockaddr)) == -1) {
            close(sock);
            throw std::runtime_error("Socket bind hatası");
        }

        memset(&gcAddr, 0, sizeof(gcAddr));
        gcAddr.sin_family = AF_INET;
        gcAddr.sin_addr.s_addr = inet_addr(target_ip);
        gcAddr.sin_port = htons(target_port);

        fcntl(sock, F_SETFL, fcntl(sock, F_GETFL, 0) | O_NONBLOCK);

        int buffer_size = 65535;
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buffer_size, sizeof(buffer_size));
    }

    ~UDPMissionDownloader() {
        close(sock);
    }

    bool downloadMission() {
        std::cout << "Görev indirme başlatılıyor..." << std::endl;
        sendMissionRequestList();
        
        bool download_complete = false;
        uint16_t mission_count = 0;
        
        while (!download_complete) {
            mavlink_message_t msg;
            if (receiveMavlinkMessage(&msg)) {
                retry_count = 0;
                
                switch (msg.msgid) {
                    case MAVLINK_MSG_ID_HEARTBEAT: {
                        // Heartbeat mesajlarını işle
                        std::cout << "Heartbeat alındı" << std::endl;
                        break;
                    }
                    
                    case MAVLINK_MSG_ID_MISSION_COUNT: {
                        mavlink_mission_count_t count;
                        mavlink_msg_mission_count_decode(&msg, &count);
                        mission_count = count.count;
                        std::cout << "Alınacak görev sayısı: " << mission_count << std::endl;
                        expected_seq = 0;
                        requestNextMissionItem();
                        break;
                    }
                    
                    case MAVLINK_MSG_ID_MISSION_ITEM_INT: {
                        mavlink_mission_item_int_t mission_item;
                        mavlink_msg_mission_item_int_decode(&msg, &mission_item);
                        
                        std::cout << "Görev item alındı - seq: " << mission_item.seq << std::endl;
                        
                        if (mission_item.seq == expected_seq) {
                            mission_items.push_back(mission_item);
                            expected_seq++;
                            
                            if (expected_seq == mission_count) {
                                sendMissionAck();
                                download_complete = true;
                                std::cout << "Tüm görevler alındı!" << std::endl;
                            } else {
                                requestNextMissionItem();
                            }
                        } else {
                            std::cout << "Sıra dışı item, beklenen: " << expected_seq 
                                    << " alınan: " << mission_item.seq << std::endl;
                            requestNextMissionItem();
                        }
                        break;
                    }
                    
                    case MAVLINK_MSG_ID_MISSION_ACK: {
                        mavlink_mission_ack_t ack;
                        mavlink_msg_mission_ack_decode(&msg, &ack);
                        if (ack.type != MAV_MISSION_ACCEPTED) {
                            std::cerr << "Görev indirme hatası: " << (int)ack.type << std::endl;
                            return false;
                        }
                        break;
                    }
                }
            }
            
            if (checkTimeout()) {
                retry_count++;
                if (retry_count >= MAX_RETRIES) {
                    std::cerr << "Maksimum yeniden deneme sayısına ulaşıldı!" << std::endl;
                    return false;
                }

                if (mission_count == 0) {
                    std::cout << "Timeout - görev listesi tekrar isteniyor (Deneme: " 
                            << retry_count << ")" << std::endl;
                    sendMissionRequestList();
                } else {
                    std::cout << "Timeout - görev item tekrar isteniyor #" << expected_seq 
                            << " (Deneme: " << retry_count << ")" << std::endl;
                    requestNextMissionItem();
                }
            }

            usleep(10000);
        }
        if (download_complete && !mission_items.empty()) {
            printMissionItems();
        }
        
        return true;
    }
    void printMissionItems() {
        std::cout << "\n=== Görev Listesi ===\n";
        
        std::cout << "JSON Format:\n";
        std::cout << "{\n  \"mission_items\": [\n";
        for (size_t i = 0; i < mission_items.size(); ++i) {
            cout<< missionItemToHumanReadable(mission_items[i])<<endl;
            std::cout << "    " << missionItemToJson(mission_items[i]);
            if (i < mission_items.size() - 1) {
                std::cout << ",";
            }
            std::cout << "\n";
        }
        std::cout << "  ]\n}\n\n";

    }
    std::string getMissionItemsJson() {
        std::stringstream jsonStream;
        
        // JSON formatında çıktı oluştur
        jsonStream << "{\"mission_items\":[";
        for (size_t i = 0; i < mission_items.size(); ++i) {
            jsonStream << missionItemToJson(mission_items[i]);
            if (i < mission_items.size() - 1) {
                jsonStream << ",";
            }
        }
        jsonStream << "]}";

        return jsonStream.str();
    }  

private:
    bool receiveMavlinkMessage(mavlink_message_t* msg) {
        socklen_t fromlen = sizeof(struct sockaddr);
        ssize_t recsize = recvfrom(sock, (void *)buf, sizeof(buf), 0, 
                                 (struct sockaddr *)&gcAddr, &fromlen);
        
        if (recsize > 0) {
            mavlink_status_t status;
            for (ssize_t i = 0; i < recsize; ++i) {
                if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], msg, &status)) {
                    return true;
                }
            }
        }
        return false;
    }

    void sendMessage(const mavlink_message_t& msg) {
        uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
        uint16_t len = mavlink_msg_to_send_buffer(buffer, &msg);
        
        ssize_t bytes_sent = sendto(sock, buffer, len, 0, 
                                  (struct sockaddr*)&gcAddr, sizeof(struct sockaddr_in));
        
        if (bytes_sent == -1) {
            std::cerr << "Mesaj gönderme hatası: " << strerror(errno) << std::endl;
        } else {
            std::cout << "Mesaj gönderildi (" << bytes_sent << " bytes)" << std::endl;
        }
    }

    void sendMissionRequestList() {
        mavlink_message_t msg;
        mavlink_mission_request_list_t request;
        request.target_system = system_id;
        request.target_component = component_id;
        
        mavlink_msg_mission_request_list_encode(system_id, component_id, &msg, &request);
        sendMessage(msg);
        resetTimeout();
    }
    
    void requestNextMissionItem() {
        mavlink_message_t msg;
        mavlink_mission_request_int_t request;
        request.target_system = system_id;
        request.target_component = component_id;
        request.seq = expected_seq;
        
        mavlink_msg_mission_request_int_encode(system_id, component_id, &msg, &request);
        sendMessage(msg);
        resetTimeout();
    }
    
    void sendMissionAck() {
        mavlink_message_t msg;
        mavlink_mission_ack_t ack;
        ack.target_system = system_id;
        ack.target_component = component_id;
        ack.type = MAV_MISSION_ACCEPTED;
        
        mavlink_msg_mission_ack_encode(system_id, component_id, &msg, &ack);
        sendMessage(msg);
    }
    
    std::chrono::steady_clock::time_point last_msg_time;
    
    void resetTimeout() {
        last_msg_time = std::chrono::steady_clock::now();
    }
    
    bool checkTimeout() {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_msg_time);
        return duration.count() > TIMEOUT_MS;
    }
};
