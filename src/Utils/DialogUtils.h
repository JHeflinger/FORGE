#pragma once
#include <string>

class Application;

class DialogUtils {
public:
    static bool Initialize(Application* app);
    static std::string FileDialog(const char* filter, bool open = true);
};