#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "proto/interface.pb.h"

namespace protobuf_experiments {

class MessageBuilder
{
public:
    static std::vector<uint8_t> buildAvailableProjectsRequest(const std::string& vin);
    static std::vector<uint8_t> buildProjectRequest(const std::string& vin, const std::string& projectName);
    static std::vector<uint8_t> buildLogsUploadRequest(const std::string& vin, const std::string& logName, const std::vector<uint8_t>& payload);
    static std::vector<uint8_t> buildFlashUploadRequest(const std::string& vin, const std::string& flashName, const std::vector<uint8_t>& payload);

private:
    static std::vector<uint8_t> serializeRequest(const Request& request);
};

}