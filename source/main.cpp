#include <fstream>
#include <iostream>

#include "json.hpp"
#include "amiibomenu.hpp"
#include "util.hpp"

#include <switch.h>

using json = nlohmann::json;
json amiibodata = json::object();

int main()
{

    consoleInit(NULL);
    socketInitializeDefault();

    if (UTIL::checkAmiiboDatabase() == true) { 
        
        std::ifstream i("sdmc:/emuiibo/amiibos.json");
        i >> amiibodata;

        AmiiboMenu menu(amiibodata);
        menu.mainLoop(); 
        
    }

    socketExit();
    consoleExit(NULL);
    return 0;
}