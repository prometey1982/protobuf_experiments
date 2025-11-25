#include "ProjectManager.hpp"
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

ProjectManager::ProjectManager(const fs::path& storagePath)
    : _storagePath(storagePath)
{
    if (!fs::exists(_storagePath)) {
        fs::create_directories(_storagePath);
    }
}

void ProjectManager::setAvailableProjects(const std::string& vin, const std::vector<std::pair<std::string, uint32_t>>& projects)
{
    std::vector<ProjectInfo> projectInfos;
    
    for (const auto& [name, serverCrc] : projects) {
        ProjectInfo info;
        info.name = name;
        info.vin = vin;
        info.crc32 = serverCrc;
        info.filePath = getProjectPath(vin, name);
        
        // Check if we already have this project with matching CRC
        if (fs::exists(info.filePath)) {
            uint32_t localCrc = calculateFileCrc32(info.filePath);
            info.isAvailable = (localCrc == serverCrc);
        } else {
            info.isAvailable = false;
        }
        
        projectInfos.push_back(info);
    }
    
    _projectsByVin[vin] = projectInfos;
}

bool ProjectManager::isProjectAvailable(const std::string& vin, const std::string& projectName) const
{
    auto it = _projectsByVin.find(vin);
    if (it == _projectsByVin.end()) {
        return false;
    }
    
    for (const auto& project : it->second) {
        if (project.name == projectName) {
            return project.isAvailable;
        }
    }
    
    return false;
}

void ProjectManager::downloadProject(const std::string& vin, const std::string& projectName, const std::vector<uint8_t>& data)
{
    fs::path projectPath = getProjectPath(vin, projectName);
    
    // Create VIN directory if it doesn't exist
    fs::create_directories(projectPath.parent_path());
    
    // Write project data to file
    std::ofstream file(projectPath, std::ios::binary);
    if (file.is_open()) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        file.close();
        
        // Update project info
        uint32_t crc = calculateCrc32(data);
        
        auto& projects = _projectsByVin[vin];
        for (auto& project : projects) {
            if (project.name == projectName) {
                project.crc32 = crc;
                project.isAvailable = true;
                break;
            }
        }
    }
}

std::vector<uint8_t> ProjectManager::getProjectData(const std::string& vin, const std::string& projectName) const
{
    fs::path projectPath = getProjectPath(vin, projectName);
    
    if (!fs::exists(projectPath)) {
        return {};
    }
    
    std::ifstream file(projectPath, std::ios::binary | std::ios::ate);
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

std::vector<std::string> ProjectManager::getAvailableProjects(const std::string& vin) const
{
    std::vector<std::string> available;
    
    auto it = _projectsByVin.find(vin);
    if (it != _projectsByVin.end()) {
        for (const auto& project : it->second) {
            if (project.isAvailable) {
                available.push_back(project.name);
            }
        }
    }
    
    return available;
}

uint32_t ProjectManager::calculateCrc32(const std::vector<uint8_t>& data) const
{
    return crc32(data.data(), data.size());
}

uint32_t ProjectManager::calculateFileCrc32(const fs::path& filePath) const
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return 0;
    }
    
    std::vector<uint8_t> buffer(std::istreambuf_iterator<char>(file), {});
    return calculateCrc32(buffer);
}

fs::path ProjectManager::getProjectPath(const std::string& vin, const std::string& projectName) const
{
    return _storagePath / vin / (projectName + ".bin");
}