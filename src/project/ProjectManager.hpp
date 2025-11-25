#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <cstdint>

namespace fs = std::filesystem;

struct ProjectInfo
{
    std::string name;
    std::string vin;
    uint32_t crc32;
    fs::path filePath;
    bool isAvailable;
};

class ProjectManager
{
public:
    ProjectManager(const fs::path& storagePath);
    
    void setAvailableProjects(const std::string& vin, const std::vector<std::pair<std::string, uint32_t>>& projects);
    bool isProjectAvailable(const std::string& vin, const std::string& projectName) const;
    void downloadProject(const std::string& vin, const std::string& projectName, const std::vector<uint8_t>& data);
    std::vector<uint8_t> getProjectData(const std::string& vin, const std::string& projectName) const;
    std::vector<std::string> getAvailableProjects(const std::string& vin) const;
    
private:
    uint32_t calculateCrc32(const std::vector<uint8_t>& data) const;
    uint32_t calculateFileCrc32(const fs::path& filePath) const;
    fs::path getProjectPath(const std::string& vin, const std::string& projectName) const;
    
    fs::path _storagePath;
    std::unordered_map<std::string, std::vector<ProjectInfo>> _projectsByVin;
};