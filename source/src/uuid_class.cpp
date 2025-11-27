


#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <random>
#include <sstream>
#include <iomanip>

// Include nlohmann/json library
#include "json.hpp"

#include "uuid_class.h"
#include "cfg.h"

using json = nlohmann::json;



std::string UuidGenerate::get_wireless_mac_address() {
    DIR *dir;
    struct dirent *entry;
    std::string mac_address;
    std::string first_mac_address;

    // 打开 /sys/class/net/ 目录
    dir = opendir("/sys/class/net/");
    if (dir == nullptr) {
        std::cerr << "Failed to open /sys/class/net/ directory" << std::endl;
        return "";
    }

    // 遍历目录中的所有条目
    while ((entry = readdir(dir)) != nullptr) {
        std::string interface_name = entry->d_name;

        // 检查接口名称是否以 wlan 或 wlp 开头
        if (interface_name.find("wlan") == 0 || interface_name.find("wlp") == 0 || interface_name.find("eth0")== 0) {   // eth0 2025.10.13
            std::string address_path = "/sys/class/net/" + interface_name + "/address";

            // 打开 address 文件
            std::ifstream address_file(address_path);
            if (address_file.is_open()) {
                std::getline(address_file, mac_address);
                address_file.close();
                closedir(dir);
                return mac_address;
            }
        } else {
            // 如果不是 wlan 或 wlp 接口，记录第一个可用的接口
            if (first_mac_address.empty()) {
                std::string address_path = "/sys/class/net/" + interface_name + "/address";

                // 打开 address 文件
                std::ifstream address_file(address_path);
                if (address_file.is_open()) {
                    std::getline(address_file, first_mac_address);
                    address_file.close();
                }
            }
        }
    }

    closedir(dir);

    // 如果没有找到 wlan 或 wlp 接口，返回第一个可用的接口的 MAC 地址
    if (!first_mac_address.empty()) {
        return first_mac_address;
    }

    return "";
}



std::string UuidGenerate::generate_uuid() {
    // 使用静态变量确保 random_device 和 mt19937 只被初始化一次
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::uniform_int_distribution<> dis2(8, 11);

    std::stringstream ss;
    int i;

    ss << std::hex;

    // 生成 UUID 的各个部分
    for (i = 0; i < 8; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    ss << dis2(gen);
    for (i = 0; i < 3; i++) {
        ss << dis(gen);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << dis(gen);
    };

    return ss.str();
}


/**
 * 从配置文件中读取 UUID
 *
 * 该函数尝试从 /etc/xiaozhi.cfg 文件中读取 UUID。
 * 如果文件存在且包含有效的 UUID，则返回该 UUID。
 * 否则，返回空字符串。
 *
 * @return 从配置文件中读取的 UUID，如果未找到则返回空字符串
 */
std::string UuidGenerate::read_uuid_from_config() {
    std::ifstream config_file(CFG_FILE);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open " CFG_FILE " for reading" << std::endl;
        return "";
    }

    try {
        json config_json;
        config_file >> config_json;
        config_file.close();

        if (config_json.contains("uuid")) {
            return config_json["uuid"].get<std::string>();
        }
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to parse " CFG_FILE ": " << e.what() << std::endl;
    }

    return "";
}

/**
 * 将 UUID 写入配置文件
 *
 * 该函数将给定的 UUID 写入 /etc/xiaozhi.cfg 文件。
 * 如果文件不存在，则创建新文件。
 *
 * @param uuid 要写入配置文件的 UUID
 * @return 成功写入文件返回 true，否则返回 false
 */
bool UuidGenerate::write_uuid_to_config(const std::string& uuid) {
    std::ofstream config_file(CFG_FILE);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open " CFG_FILE " for writing" << std::endl;
        return false;
    }

    try {
        json config_json;
        config_json["uuid"] = uuid;
        config_file << config_json.dump(4); // 使用 4 个空格进行缩进
        config_file.close();
        return true;
    } catch (const nlohmann::json::parse_error& e) {
        std::cerr << "Failed to write to " CFG_FILE ": " << e.what() << std::endl;
    }

    return false;
}


void UuidGenerate::httpParam_init(){
    // 获取无线网卡的 MAC 地址
    mac = get_wireless_mac_address();   //get_wireless_mac_address();  //uuidCreate.get_wireless_mac_address();  
    if (mac.empty()) {
        std::cerr << "Failed to get wireless MAC address" << std::endl;
        mac = "00:00:00:00:00:00"; // 默认值，可以根据需要修改
    }
    std::cout << "### mac : " << mac<<  std::endl;

    // 读取配置文件中的 UUID
    uuid = read_uuid_from_config();
    if (uuid.empty()) {
        std::cerr << "UUID not found in " CFG_FILE << std::endl;
        // 生成新的 UUID
        uuid = generate_uuid();  //generate_uuid();  // uuidCreate.generate_uuid();
        std::cout << "Generated new UUID: " << uuid << std::endl;

        // 将新的 UUID 写入配置文件
        if (!write_uuid_to_config(uuid)) {
            std::cerr << "Failed to write UUID to " CFG_FILE << std::endl;
        } else {
            std::cout << "UUID written to " CFG_FILE << std::endl;
        }
    } else {
        std::cout << "UUID from " CFG_FILE ": " << uuid << std::endl;
    }   

    // http_data_t http_data;
    http_data.url = "https://api.tenclass.net/xiaozhi/ota/";

    // 替换 http_data.post 中的 uuid
    std::ostringstream post_stream;
    post_stream << R"(
        {
            "uuid":")" << uuid << R"(",
            "application": {
                "name": "xiaozhi_linux_100ask", 
                "version": "1.0.0"
            },
            "ota": {
            },
            "board": {
                "type": "100ask_linux_board", 
                "name": "100ask_imx6ull_board" 
            }
        }
    )";
    http_data.post = post_stream.str();

    // 替换 http_data.headers 中的 Device-Id
    std::ostringstream headers_stream;
    headers_stream << R"(
        {
            "Content-Type": "application/json",
            "Device-Id": ")" << mac << R"(",
            "User-Agent": "weidongshan1",
            "Accept-Language": "zh-CN"
        }
    )";
    http_data.headers = headers_stream.str();    
}


