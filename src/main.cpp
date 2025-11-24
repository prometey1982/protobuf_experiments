#include <iostream>
#include <thread>
#include <chrono>
#include "client/client.hpp"
#include "project/project_manager.hpp"

int main(int argc, char *argv[])
{
    try {
        // Параметры подключения
        std::string host = "127.0.0.1";
        std::string port = "8080";
        std::string path = "/wss";
        std::string storage_path = "./projects";
        
        // Создание клиента
        Client client(host, port, path, storage_path);

        // Установка callback функций
        client.set_available_projects_callback([&client](const std::string& vin, const std::vector<std::string>& projects) {
            std::cout << "Available projects for VIN " << vin << ":" << std::endl;
            for (const auto& project : projects) {
                std::cout << "  - " << project << std::endl;
            }
        });

        client.set_project_received_callback([](const std::string& vin, const std::string& project_name, bool success) {
            if (success) {
                std::cout << "Project " << project_name << " received successfully for VIN " << vin << std::endl;
            } else {
                std::cout << "Failed to receive project " << project_name << " for VIN " << vin << std::endl;
            }
        });
        
        client.set_upload_complete_callback([](const std::string& vin, const std::string& name, bool success) {
            if (success) {
                std::cout << "Upload completed successfully for " << name << " (VIN: " << vin << ")" << std::endl;
            } else {
                std::cout << "Upload failed for " << name << " (VIN: " << vin << ")" << std::endl;
            }
        });
        
        // Подключение к серверу
        std::cout << "Connecting to server..." << std::endl;
        client.connect();
        
        // Даем время на установку соединения
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Демонстрация функционала
        std::string vin = "myvin";
        
        // Запрос доступных проектов
        std::cout << "\nRequesting available projects..." << std::endl;
        client.request_available_projects(vin);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        std::set<std::string> projectsToDownload;
        for(const auto& projectName: client.get_project_manager().get_available_projects(vin)) {
            projectsToDownload.insert(projectName);
        }
        // Запрос проекта
        for(const auto& project: projectsToDownload) {
            std::cout << "\nRequesting project..." << std::endl;
            client.request_project(vin, project);
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Загрузка логов (пример)
        std::cout << "\nUploading logs..." << std::endl;
        std::vector<uint8_t> log_data = {0x10, 0x20, 0x30, 0x40};
        client.upload_logs(vin, "log1", log_data);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // Загрузка прошивки (пример)
        std::cout << "\nUploading flash..." << std::endl;
        std::vector<uint8_t> flash_data = {0x50, 0x60, 0x70, 0x80};
        client.upload_flash(vin, "flash1", flash_data);
        
        // Ожидание завершения операций
        std::this_thread::sleep_for(std::chrono::seconds(3));
        
        std::cout << "\nDemo completed. Disconnecting..." << std::endl;
        client.disconnect();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
