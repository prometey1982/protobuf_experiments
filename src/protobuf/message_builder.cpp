#include "message_builder.hpp"
#include <iostream>

namespace protobuf_experiments {

std::vector<uint8_t> MessageBuilder::build_available_projects_request(const std::string& vin) {
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_AVAILABLE_PROJECTS);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    request.mutable_available_projects();
    // Для запроса доступных проектов полезная нагрузка пустая
    
    return serialize_request(request);
}

std::vector<uint8_t> MessageBuilder::build_project_request(const std::string& vin, const std::string& project_name) {
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_PROJECT);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    ProjectRequest* project_request = request.mutable_project();
    project_request->set_name(project_name);
    
    return serialize_request(request);
}

std::vector<uint8_t> MessageBuilder::build_logs_upload_request(const std::string& vin, const std::string& log_name, const std::vector<uint8_t>& payload) {
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_LOGS_UPLOAD);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    LogsUploadRequest* logs_upload = request.mutable_logs_upload();
    logs_upload->set_name(log_name);
    logs_upload->set_payload(std::string(reinterpret_cast<const char*>(payload.data()), payload.size()));
    
    return serialize_request(request);
}

std::vector<uint8_t> MessageBuilder::build_flash_upload_request(const std::string& vin, const std::string& flash_name, const std::vector<uint8_t>& payload) {
    Request request;
    
    // Заполняем заголовок
    RequestHeader* header = request.mutable_header();
    header->set_version(1);
    header->set_request_type(REQ_FLASH_UPLOAD);
    header->set_vin(vin);
    
    // Заполняем полезную нагрузку
    FlashUploadRequest* flash_upload = request.mutable_flash_upload();
    flash_upload->set_name(flash_name);
    flash_upload->set_payload(std::string(reinterpret_cast<const char*>(payload.data()), payload.size()));
    
    return serialize_request(request);
}

std::vector<uint8_t> MessageBuilder::serialize_request(const Request& request) {
    std::vector<uint8_t> serialized(request.ByteSizeLong());
    if (!request.SerializeToArray(serialized.data(), serialized.size())) {
        std::cerr << "Failed to serialize protobuf request" << std::endl;
        return {};
    }
    return serialized;
}

}