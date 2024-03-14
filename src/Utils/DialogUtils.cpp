#include "DialogUtils.h"
#include "../Core/Safety.h"
#include "../Core/Window.h"
#include "../Core/Application.h"

static Ref<Window> s_Window;

bool DialogUtils::Initialize(Application* app) {
    if (app) {
        s_Window = app->GetWindow();
        if (s_Window) return true;
    }
    return false;
}

#ifdef WINDOWS_BUILD

#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <Windows.h>
#include <shlobj.h>

std::string DialogUtils::FileDialog(const char* filter, bool open) {
    OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)s_Window->GetNativeWindow());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (open) {
	    if (GetOpenFileNameA(&ofn) == TRUE)
		    return ofn.lpstrFile;
    } else {
	    if (GetSaveFileNameA(&ofn) == TRUE)
		    return ofn.lpstrFile;
    }
	return std::string();
}

#elif LINUX_BUILD

std::string DialogUtils::FileDialog(const char* filter, bool open) {
    char filename[2048];
    FILE *f = open ? popen("zenity --file-selection --file-filter=*.fsim", "r") : popen("zenity --file-selection --save --file-filter=*.fsim", "r");
    fgets(filename, 2048, f);
	for (int i = 0; i < 2048; i++) if (filename[i] == '\n') filename[i] = '\0';
    pclose(f);
	return std::string(filename);
}

#else
#error "Only windows and linux builds are currently supported!"
#endif
