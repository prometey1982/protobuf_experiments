#include <iostream>
#include <thread>
#include <chrono>
#include "client/client.hpp"
#include "project/ProjectManager.hpp"

int main(int argc, char *argv[])
{
    try {
        // Параметры подключения
        std::string host = "127.0.0.1";
        std::string port = "8080";
        std::string path = "/wss";
        std::string storagePath = "./projects";
        
                // Создание клиента
        Client client(host, port, path, storagePath);

                // Установка callback функций
        client.setAvailableProjectsCallback([&client](const std::string& vin, const std::vector<std::string>& projects) {
            std::cout << "Available projects for VIN " << vin << ":" << std::endl;
            for (const auto& project : projects) {
                std::cout << "  - " << project << std::endl;
            }
        });

        client.setProjectReceivedCallback([](const std::string& vin, const std::string& projectName, bool success) {
            if (success) {
                std::cout << "Project " << projectName << " received successfully for VIN " << vin << std::endl;
            } else {
                std::cout << "Failed to receive project " << projectName << " for VIN " << vin << std::endl;
            }
        });
        
        client.setUploadCompleteCallback([](const std::string& vin, const std::string& name, bool success) {
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
        client.requestAvailableProjects(vin);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
                std::set<std::string> projectsToDownload;
        for(const auto& projectName: client.getProjectManager().getAvailableProjects(vin)) {
            projectsToDownload.insert(projectName);
        }
        // Запрос проекта
        for(const auto& project: projectsToDownload) {
            std::cout << "\nRequesting project..." << std::endl;
            client.requestProject(vin, project);
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
                // Загрузка логов (пример)
        std::cout << "\nUploading logs..." << std::endl;
        std::vector<uint8_t> logData = {0x10, 0x20, 0x30, 0x40};
        client.uploadLogs(vin, "log1", logData);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
                // Загрузка прошивки (пример)
        std::cout << "\nUploading flash..." << std::endl;
        std::vector<uint8_t> flashData = {0x50, 0x60, 0x70, 0x80};
        client.uploadFlash(vin, "flash1", flashData);
        
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