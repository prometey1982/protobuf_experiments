#include "client.hpp"
#include <iostream>
#include "proto/interface.pb.h"

using namespace protobuf_experiments;

Client::Client(const std::string& host, const std::string& port, const std::string& path, const std::string& storage_path)
    : host_(host)
    , port_(port)
    , path_(path)
    , ssl_ctx_(ssl::context::tlsv12_client)
{
    // Настройка SSL контекста
    ssl_ctx_.set_default_verify_paths();
    ssl_ctx_.set_verify_mode(ssl::verify_peer);
    
    // Загружаем публичный ключ из srv_pub.pem
    try {
        ssl_ctx_.load_verify_file("c:/misc/programming/protobuf_experiments/python/src/srv_pub.pem");
    } catch (const std::exception& e) {
        std::cerr << "Failed to load SSL certificate: " << e.what() << std::endl;
        // Продолжаем без проверки сертификата для демонстрации
        ssl_ctx_.set_verify_mode(ssl::verify_none);
    }
    
    // Создание менеджера проектов
    project_manager_ = std::make_unique<ProjectManager>(storage_path);
    
    // Создание WebSocket клиента
    ws_client_ = std::make_shared<WebSocketClient>(ioc_, ssl_ctx_);
    ws_client_->set_message_handler([this](const std::string& message) {
        this->handle_message(message);
    });
}

Client::~Client() {
    disconnect();
}

void Client::connect() {
    ws_client_->connect(host_, port_, path_);
    
    // Запуск io_context в отдельном потоке для асинхронной работы
    std::thread([this]() {
        ioc_.run();
    }).detach();
}

void Client::disconnect() {
    if (ws_client_) {
        ws_client_->close();
    }
    ioc_.stop();
}

ProjectManager& Client::get_project_manager() {
    return *project_manager_;
}

const ProjectManager& Client::get_project_manager() const {
    return *project_manager_;
}

void Client::request_available_projects(const std::string& vin) {
    auto message = protobuf_experiments::MessageBuilder::build_available_projects_request(vin);
    ws_client_->send(message);
}

void Client::request_project(const std::string& vin, const std::string& project_name) {
    // Проверяем, есть ли проект локально
    if (project_manager_->is_project_available(vin, project_name)) {
        std::cout << "Project " << project_name << " is already available locally" << std::endl;
        if (project_received_callback_) {
            project_received_callback_(vin, project_name, true);
        }
        return;
    }
    
    // Если проекта нет, запрашиваем с сервера
    auto message = protobuf_experiments::MessageBuilder::build_project_request(vin, project_name);
    ws_client_->send(message);
}

void Client::upload_logs(const std::string& vin, const std::string& log_name, const std::vector<uint8_t>& log_data) {
    auto message = protobuf_experiments::MessageBuilder::build_logs_upload_request(vin, log_name, log_data);
    ws_client_->send(message);
}

void Client::upload_flash(const std::string& vin, const std::string& flash_name, const std::vector<uint8_t>& flash_data) {
    auto message = protobuf_experiments::MessageBuilder::build_flash_upload_request(vin, flash_name, flash_data);
    ws_client_->send(message);
}

void Client::set_available_projects_callback(std::function<void(const std::string&, const std::vector<std::string>&)> callback) {
    available_projects_callback_ = callback;
}

void Client::set_project_received_callback(std::function<void(const std::string&, const std::string&, bool)> callback) {
    project_received_callback_ = callback;
}

void Client::set_upload_complete_callback(std::function<void(const std::string&, const std::string&, bool)> callback) {
    upload_complete_callback_ = callback;
}

void Client::handle_message(const std::string& message) {
    std::cout << "Received message of size: " << message.size() << std::endl;
    
    // Парсинг protobuf сообщения
    Response response;
    if (!response.ParseFromString(message)) {
        std::cerr << "Failed to parse protobuf response" << std::endl;
        return;
    }
    
    const ResponseHeader& header = response.header();
    std::string vin = header.vin();
    
    switch (header.response_type()) {
        case RESP_AVAILABLE_PROJECTS:
            process_available_projects_response(vin, response);
            break;
        case RESP_PROJECT:
            process_project_response(vin, response);
            break;
        case RESP_LOGS_UPLOAD:
            if (upload_complete_callback_) {
                upload_complete_callback_(vin, "logs", true);
            }
            break;
        case RESP_FLASH_UPLOAD:
            if (upload_complete_callback_) {
                upload_complete_callback_(vin, "flash", true);
            }
            break;
        case RESP_ERROR:
            std::cerr << "Received error response" << std::endl;
            break;
        default:
            std::cerr << "Unknown response type: " << header.response_type() << std::endl;
            break;
    }
}

void Client::process_available_projects_response(const std::string& vin, const Response& response) {
    if (!response.has_available_projects()) {
        std::cerr << "Available projects response missing payload" << std::endl;
        return;
    }
    
    const AvailableProjectsResponse& available_projects_response = response.available_projects();
    std::vector<std::pair<std::string, uint32_t>> projects;
    
    for (int i = 0; i < available_projects_response.available_projects_size(); ++i) {
        const AvailableProject& project = available_projects_response.available_projects(i);
        projects.emplace_back(project.name(), project.crc());
    }
    
    project_manager_->set_available_projects(vin, projects);
    
    if (available_projects_callback_) {
        std::vector<std::string> project_names;
        for (const auto& project : projects) {
            project_names.push_back(project.first);
        }
        available_projects_callback_(vin, project_names);
    }
}

void Client::process_project_response(const std::string& vin, const Response& response) {
    if (!response.has_project()) {
        std::cerr << "Project response missing payload" << std::endl;
        return;
    }
    
    const ProjectResponse& project_response = response.project();
    const std::string& project_data = project_response.data();
    std::vector<std::string> available_projects = project_manager_->get_available_projects(vin);
    std::vector<uint8_t> data(project_data.begin(), project_data.end());
    project_manager_->download_project(vin, project_response.name(), data);
}
