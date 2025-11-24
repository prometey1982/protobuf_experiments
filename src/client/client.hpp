#pragma once

#include "../websocket/client.hpp"
#include "../protobuf/message_builder.hpp"
#include "../project/project_manager.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "proto/interface.pb.h"

class Client {
public:
    Client(const std::string& host, const std::string& port, const std::string& path, const std::string& storage_path);
    ~Client();
    
    void connect();
    void disconnect();

    ProjectManager& get_project_manager();
    const ProjectManager& get_project_manager() const;
    
    // Основные методы согласно ТЗ
    void request_available_projects(const std::string& vin);
    void request_project(const std::string& vin, const std::string& project_name);
    void upload_logs(const std::string& vin, const std::string& log_name, const std::vector<uint8_t>& log_data);
    void upload_flash(const std::string& vin, const std::string& flash_name, const std::vector<uint8_t>& flash_data);
    
    // Callback функции
    void set_available_projects_callback(std::function<void(const std::string& vin, const std::vector<std::string>& projects)> callback);
    void set_project_received_callback(std::function<void(const std::string& vin, const std::string& project_name, bool success)> callback);
    void set_upload_complete_callback(std::function<void(const std::string& vin, const std::string& name, bool success)> callback);
    
private:
    void handle_message(const std::string& message);
    void process_available_projects_response(const std::string& vin, const protobuf_experiments::Response& response);
    void process_project_response(const std::string& vin, const protobuf_experiments::Response& response);
    
    std::string host_;
    std::string port_;
    std::string path_;
    
    std::shared_ptr<WebSocketClient> ws_client_;
    std::unique_ptr<ProjectManager> project_manager_;
    
    // Callbacks
    std::function<void(const std::string&, const std::vector<std::string>&)> available_projects_callback_;
    std::function<void(const std::string&, const std::string&, bool)> project_received_callback_;
    std::function<void(const std::string&, const std::string&, bool)> upload_complete_callback_;
    
    net::io_context ioc_;
    ssl::context ssl_ctx_;
};