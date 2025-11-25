#include "client.hpp"
#include <iostream>
#include "proto/interface.pb.h"

Client::Client(const std::string& host, const std::string& port, const std::string& path, const std::string& storagePath)
    : _host(host)
    , _port(port)
    , _path(path)
    , _sslCtx(ssl::context::tlsv12_client)
{
    // Настройка SSL контекста
    _sslCtx.set_default_verify_paths();
    _sslCtx.set_verify_mode(ssl::verify_peer);
    
    // Загружаем публичный ключ из srv_pub.pem
    try {
        _sslCtx.load_verify_file("python/src/srv_pub.pem");
    } catch (const std::exception& e) {
        std::cerr << "Failed to load SSL certificate: " << e.what() << std::endl;
        // Продолжаем без проверки сертификата для демонстрации
        _sslCtx.set_verify_mode(ssl::verify_none);
    }
    
    // Создание менеджера проектов
    _projectManager = std::make_unique<ProjectManager>(storagePath);
    
    // Создание WebSocket клиента
    _wsClient = std::make_unique<WebSocketClient>(_ioc, _sslCtx);
    _wsClient->setMessageHandler([this](const std::string& message) {
        this->handleMessage(message);
    });
}

Client::~Client()
{
    disconnect();
}

void Client::connect()
{
    _wsClient->connect(_host, _port, _path);
    
    // Запуск io_context в отдельном потоке для асинхронной работы
    std::thread([this]() {
        _ioc.run();
    }).detach();
}

void Client::disconnect()
{
    if (_wsClient) {
        _wsClient->close();
    }
    _ioc.stop();
}

ProjectManager& Client::getProjectManager()
{
    return *_projectManager;
}

const ProjectManager& Client::getProjectManager() const
{
    return *_projectManager;
}

void Client::requestAvailableProjects(const std::string& vin)
{
    auto message = protobuf_experiments::MessageBuilder::buildAvailableProjectsRequest(vin);
    _wsClient->send(message);
}

void Client::requestProject(const std::string& vin, const std::string& projectName)
{
    // Проверяем, есть ли проект локально
    if (_projectManager->isProjectAvailable(vin, projectName)) {
        std::cout << "Project " << projectName << " is already available locally" << std::endl;
        if (_projectReceivedCallback) {
            _projectReceivedCallback(vin, projectName, true);
        }
        return;
    }
    
    // Если проекта нет, запрашиваем с сервера
    auto message = protobuf_experiments::MessageBuilder::buildProjectRequest(vin, projectName);
    _wsClient->send(message);
}

void Client::uploadLogs(const std::string& vin, const std::string& logName, const std::vector<uint8_t>& logData)
{
    auto message = protobuf_experiments::MessageBuilder::buildLogsUploadRequest(vin, logName, logData);
    _wsClient->send(message);
}

void Client::uploadFlash(const std::string& vin, const std::string& flashName, const std::vector<uint8_t>& flashData)
{
    auto message = protobuf_experiments::MessageBuilder::buildFlashUploadRequest(vin, flashName, flashData);
    _wsClient->send(message);
}

void Client::setAvailableProjectsCallback(std::function<void(const std::string&, const std::vector<std::string>&)> callback)
{
    _availableProjectsCallback = callback;
}

void Client::setProjectReceivedCallback(std::function<void(const std::string&, const std::string&, bool)> callback)
{
    _projectReceivedCallback = callback;
}

void Client::setUploadCompleteCallback(std::function<void(const std::string&, const std::string&, bool)> callback)
{
    _uploadCompleteCallback = callback;
}

void Client::handleMessage(const std::string& message)
{
    std::cout << "Received message of size: " << message.size() << std::endl;
    
    // Парсинг protobuf сообщения
    protobuf_experiments::Response response;
    if (!response.ParseFromString(message)) {
        std::cerr << "Failed to parse protobuf response" << std::endl;
        return;
    }
    
    const protobuf_experiments::ResponseHeader& header = response.header();
    std::string vin = header.vin();
    
    switch (header.response_type()) {
        case protobuf_experiments::RESP_AVAILABLE_PROJECTS:
            processAvailableProjectsResponse(vin, response);
            break;
        case protobuf_experiments::RESP_PROJECT:
            processProjectResponse(vin, response);
            break;
        case protobuf_experiments::RESP_LOGS_UPLOAD:
            if (_uploadCompleteCallback) {
                _uploadCompleteCallback(vin, "logs", true);
            }
            break;
        case protobuf_experiments::RESP_FLASH_UPLOAD:
            if (_uploadCompleteCallback) {
                _uploadCompleteCallback(vin, "flash", true);
            }
            break;
        case protobuf_experiments::RESP_ERROR:
            std::cerr << "Received error response" << std::endl;
            break;
        default:
            std::cerr << "Unknown response type: " << header.response_type() << std::endl;
            break;
    }
}

void Client::processAvailableProjectsResponse(const std::string& vin, const protobuf_experiments::Response& response)
{
    if (!response.has_available_projects()) {
        std::cerr << "Available projects response missing payload" << std::endl;
        return;
    }
    
    const protobuf_experiments::AvailableProjectsResponse& availableProjectsResponse = response.available_projects();
    std::vector<std::pair<std::string, uint32_t>> projects;
    
    for (int i = 0; i < availableProjectsResponse.available_projects_size(); ++i) {
        const protobuf_experiments::AvailableProject& project = availableProjectsResponse.available_projects(i);
        projects.emplace_back(project.name(), project.crc());
    }
    
    _projectManager->setAvailableProjects(vin, projects);
    
    if (_availableProjectsCallback) {
        std::vector<std::string> projectNames;
        for (const auto& project : projects) {
            projectNames.push_back(project.first);
        }
        _availableProjectsCallback(vin, projectNames);
    }
}

void Client::processProjectResponse(const std::string& vin, const protobuf_experiments::Response& response)
{
    if (!response.has_project()) {
        std::cerr << "Project response missing payload" << std::endl;
        return;
    }
    
    const protobuf_experiments::ProjectResponse& projectResponse = response.project();
    const std::string& projectData = projectResponse.data();
    std::vector<uint8_t> data(projectData.begin(), projectData.end());
    _projectManager->downloadProject(vin, projectResponse.name(), data);
    
    if (_projectReceivedCallback) {
        _projectReceivedCallback(vin, projectResponse.name(), true);
    }
}