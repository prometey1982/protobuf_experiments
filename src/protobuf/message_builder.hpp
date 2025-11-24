#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "proto/interface.pb.h"

namespace protobuf_experiments {

class MessageBuilder {
public:
    static std::vector<uint8_t> build_available_projects_request(const std::string& vin);
    static std::vector<uint8_t> build_project_request(const std::string& vin, const std::string& project_name);
    static std::vector<uint8_t> build_logs_upload_request(const std::string& vin, const std::string& log_name, const std::vector<uint8_t>& payload);
    static std::vector<uint8_t> build_flash_upload_request(const std::string& vin, const std::string& flash_name, const std::vector<uint8_t>& payload);

private:
    static std::vector<uint8_t> serialize_request(const Request& request);
};

}