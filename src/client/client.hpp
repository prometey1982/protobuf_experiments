#pragma once

#include "websocket/WebSocketClient.hpp"
#include "protobuf/message_builder.hpp"
#include "project/ProjectManager.hpp"
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include "proto/interface.pb.h"

class Client {
public:
    Client(const std::string& host, const std::string& port, const std::string& path, const std::string& storagePath);
    ~Client();
    
    void connect();
    void disconnect();

    ProjectManager& getProjectManager();
    const ProjectManager& getProjectManager() const;
    
    // Основные методы согласно ТЗ
    void requestAvailableProjects(const std::string& vin);
    void requestProject(const std::string& vin, const std::string& projectName);
    void uploadLogs(const std::string& vin, const std::string& logName, const std::vector<uint8_t>& logData);
    void uploadFlash(const std::string& vin, const std::string& flashName, const std::vector<uint8_t>& flashData);
    
    // Callback функции
    void setAvailableProjectsCallback(std::function<void(const std::string& vin, const std::vector<std::string>& projects)> callback);
    void setProjectReceivedCallback(std::function<void(const std::string& vin, const std::string& projectName, bool success)> callback);
    void setUploadCompleteCallback(std::function<void(const std::string& vin, const std::string& name, bool success)> callback);
    
private:
    void handleMessage(const std::string& message);
    void processAvailableProjectsResponse(const std::string& vin, const protobuf_experiments::Response& response);
    void processProjectResponse(const std::string& vin, const protobuf_experiments::Response& response);
    
    std::string _host;
    std::string _port;
    std::string _path;
    
    std::shared_ptr<WebSocketClient> _wsClient;
    std::unique_ptr<ProjectManager> _projectManager;
    
    // Callbacks
    std::function<void(const std::string&, const std::vector<std::string>&)> _availableProjectsCallback;
    std::function<void(const std::string&, const std::string&, bool)> _projectReceivedCallback;
    std::function<void(const std::string&, const std::string&, bool)> _uploadCompleteCallback;
    
    net::io_context _ioc;
    ssl::context _sslCtx;
};