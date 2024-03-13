#pragma once
#include <string>

class Application;

class DialogUtils {
public:
    static bool DialogUtils::Initialize(Application* app);
    static std::string FileDialog(const char* filter);
};