#include "App.h"

#include <string>

int main(int argc, char** argv)
{
    bool smokeTest = argc > 1 && std::string(argv[1]) == "--smoke-test";
    App app;
    return app.run(smokeTest);
}
