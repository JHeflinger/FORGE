#pragma once
#include <string>

class FileUtils {
public:
    static void Write(const std::string& content, const std::string& filepath);
    static std::string Read(const std::string& filepath);
    static bool Exists(const std::string& filepath);
    static std::string Save(const std::string& content, const std::string& filepath = "");
    static std::string Open();
};