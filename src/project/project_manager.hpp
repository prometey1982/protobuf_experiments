#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <filesystem>
#include <cstdint>

namespace fs = std::filesystem;

struct ProjectInfo {
    std::string name;
    std::string vin;
    uint32_t crc32;
    fs::path file_path;
    bool is_available;
};

class ProjectManager {
public:
    ProjectManager(const fs::path& storage_path);
    
    void set_available_projects(const std::string& vin, const std::vector<std::pair<std::string, uint32_t>>& projects);
    bool is_project_available(const std::string& vin, const std::string& project_name) const;
    void download_project(const std::string& vin, const std::string& project_name, const std::vector<uint8_t>& data);
    std::vector<uint8_t> get_project_data(const std::string& vin, const std::string& project_name) const;
    std::vector<std::string> get_available_projects(const std::string& vin) const;
    
private:
    uint32_t calculate_crc32(const std::vector<uint8_t>& data) const;
    uint32_t calculate_file_crc32(const fs::path& file_path) const;
    fs::path get_project_path(const std::string& vin, const std::string& project_name) const;
    
    fs::path storage_path_;
    std::unordered_map<std::string, std::vector<ProjectInfo>> projects_by_vin_;
};