#include "project_manager.hpp"
#include <boost/crc.hpp>
#include <fstream>
#include <iostream>

namespace {

uint32_t crc32(const void* data, size_t dataSize) {
    boost::crc_32_type result;
    result.process_bytes(data, dataSize);
    return result.checksum();
}    

}

ProjectManager::ProjectManager(const fs::path& storage_path)
    : storage_path_(storage_path)
{
    if (!fs::exists(storage_path_)) {
        fs::create_directories(storage_path_);
    }
}

void ProjectManager::set_available_projects(const std::string& vin, const std::vector<std::pair<std::string, uint32_t>>& projects) {
    std::vector<ProjectInfo> project_infos;
    
    for (const auto& [name, server_crc] : projects) {
        ProjectInfo info;
        info.name = name;
        info.vin = vin;
        info.crc32 = server_crc;
        info.file_path = get_project_path(vin, name);
        
        // Check if we already have this project with matching CRC
        if (fs::exists(info.file_path)) {
            uint32_t local_crc = calculate_file_crc32(info.file_path);
            info.is_available = (local_crc == server_crc);
        } else {
            info.is_available = false;
        }
        
        project_infos.push_back(info);
    }
    
    projects_by_vin_[vin] = project_infos;
}

bool ProjectManager::is_project_available(const std::string& vin, const std::string& project_name) const {
    auto it = projects_by_vin_.find(vin);
    if (it == projects_by_vin_.end()) {
        return false;
    }
    
    for (const auto& project : it->second) {
        if (project.name == project_name) {
            return project.is_available;
        }
    }
    
    return false;
}

void ProjectManager::download_project(const std::string& vin, const std::string& project_name, const std::vector<uint8_t>& data) {
    fs::path project_path = get_project_path(vin, project_name);
    
    // Create VIN directory if it doesn't exist
    fs::create_directories(project_path.parent_path());
    
    // Write project data to file
    std::ofstream file(project_path, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        
        // Update project info
        uint32_t crc = calculate_crc32(data);
        
        auto& projects = projects_by_vin_[vin];
        for (auto& project : projects) {
            if (project.name == project_name) {
                project.crc32 = crc;
                project.is_available = true;
                break;
            }
        }
    }
}

std::vector<uint8_t> ProjectManager::get_project_data(const std::string& vin, const std::string& project_name) const {
    fs::path project_path = get_project_path(vin, project_name);
    
    if (!fs::exists(project_path)) {
        return {};
    }
    
    std::ifstream file(project_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return {};
    }
    
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(size);
    if (file.read(reinterpret_cast<char*>(data.data()), size)) {
        return data;
    }
    
    return {};
}

std::vector<std::string> ProjectManager::get_available_projects(const std::string& vin) const {
    std::vector<std::string> available;
    
    auto it = projects_by_vin_.find(vin);
    if (it != projects_by_vin_.end()) {
        for (const auto& project : it->second) {
            if (project.is_available) {
                available.push_back(project.name);
            }
        }
    }
    
    return available;
}

uint32_t ProjectManager::calculate_crc32(const std::vector<uint8_t>& data) const {
    return crc32(data.data(), data.size());
}

uint32_t ProjectManager::calculate_file_crc32(const fs::path& file_path) const {
    std::ifstream file(file_path, std::ios::binary);
    if (!file.is_open()) {
        return 0;
    }
    
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return calculate_crc32(buffer);
}

fs::path ProjectManager::get_project_path(const std::string& vin, const std::string& project_name) const {
    return storage_path_ / vin / (project_name + ".bin");
}