#include "Core/Application.h"

int main(int, char**) {
    Application app;
    if (app.Initialize()) {
        app.Run();
        app.Shutdown();
        return 0;
    }
    return 1;
}
