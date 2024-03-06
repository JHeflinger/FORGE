#pragma once
#include <string>

class FileUtils {
    static void Write(const std::string& content, const std::string& filepath);
    static std::string Read(const std::string& filepath);
};