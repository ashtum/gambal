#include "config.hpp"
#include "gui.hpp"
#include "proc.hpp"

#include <cstdio>
#include <iostream>

#include <sys/stat.h>

auto init_config_path()
{
    auto home_dotconfig_path = std::string{ getenv("HOME") } + "/.config";
    auto dir_path = home_dotconfig_path + "/gambal";
    mkdir(dir_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return dir_path + "/config";
}

int main()
{
    try
    {
        gambal::config config{ init_config_path() };
        gambal::proc proc{ &config };
        gambal::gui gui{ &config, &proc };
        gui.run();
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Exit with exception: " << ex.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}