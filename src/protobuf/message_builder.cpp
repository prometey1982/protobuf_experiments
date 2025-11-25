#include "message_builder.hpp"
#include <iostream>

namespace protobuf_experiments {

std::vector<uint8_t> MessageBuilder::buildAvailableProjectsRequest(const std::string& vin)
{
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_AVAILABLE_PROJECTS);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    request.mutable_available_projects();
    // Для запроса доступных проектов полезная нагрузка пустая
    
    return serializeRequest(request);
}

std::vector<uint8_t> MessageBuilder::buildProjectRequest(const std::string& vin, const std::string& projectName)
{
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_PROJECT);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    ProjectRequest* projectRequest = request.mutable_project();
    projectRequest->set_name(projectName);
    
    return serializeRequest(request);
}

std::vector<uint8_t> MessageBuilder::buildLogsUploadRequest(const std::string& vin, const std::string& logName, const std::vector<uint8_t>& payload)
{
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_LOGS_UPLOAD);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    LogsUploadRequest* logsUpload = request.mutable_logs_upload();
    logsUpload->set_name(logName);
    logsUpload->set_payload(std::string(reinterpret_cast<const char*>(payload.data()), payload.size()));
    
    return serializeRequest(request);
}

std::vector<uint8_t> MessageBuilder::buildFlashUploadRequest(const std::string& vin, const std::string& flashName, const std::vector<uint8_t>& payload)
{
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_FLASH_UPLOAD);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    FlashUploadRequest* flashUpload = request.mutable_flash_upload();
    flashUpload->set_name(flashName);
    flashUpload->set_payload(std::string(reinterpret_cast<const char*>(payload.data()), payload.size()));
    
    return serializeRequest(request);
}

std::vector<uint8_t> MessageBuilder::serializeRequest(const Request& request)
{
    std::vector<uint8_t> serialized(request.ByteSizeLong());
    if (!request.SerializeToArray(serialized.data(), serialized.size())) {
        std::cerr << "Failed to serialize protobuf request" << std::endl;
        return {};
    }
    return serialized;
}

}