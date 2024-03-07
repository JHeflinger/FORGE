#include "FileUtils.h"
#include "../Core/Log.h"
#include <iostream>
#include <fstream>
#include <sstream>

void FileUtils::Write(const std::string& content, const std::string& filepath) {
    std::ofstream outputFile(filepath);
    if (outputFile.is_open()) {
        outputFile << content;
        outputFile.close();
    } else FATAL("Could not write content to file \"{}\"", filepath);
}

std::string FileUtils::Read(const std::string& filepath) {
    std::ifstream inputFile(filepath);
    std::stringstream buffer;
    if (inputFile.is_open()) {
        buffer << inputFile.rdbuf();
        inputFile.close();
        return buffer.str();
    } else FATAL("Could not read content of file \"{}\"", filepath);
    return "";
}

bool FileUtils::Exists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}