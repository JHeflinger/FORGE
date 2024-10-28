#include "FileUtils.h"
#include "Utils/DialogUtils.h"
#include "Core/Log.h"
#include <iostream>
#include <fstream>
#include <sstream>

static bool EndsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length())
        return false;
    return str.substr(str.length() - suffix.length()) == suffix;
}

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

std::string FileUtils::Save(const std::string& content, const std::string& filepath) {
    std::string file = filepath;
    if (file == "") file = DialogUtils::FileDialog("Simulation (*.fsim)\0*.fsim\0", false);
    if (file != "") {
        if (!EndsWith(file, ".fsim")) file += ".fsim";
        Write(content, file);
        return file;
    } else FATAL("Unable to save file to disk!");
    return "";
}

std::string FileUtils::Open() {
    std::string file = DialogUtils::FileDialog("Simulation (*.fsim)\0*.fsim\0");
    if (file != "") {
        return file;
    } else FATAL("Unable to save file to disk!");
    return "";
}
