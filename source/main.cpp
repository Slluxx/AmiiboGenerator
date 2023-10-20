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
    // setsysInitialize();
    appletInitialize();
    socketInitializeDefault();

    // appletLockExit(); // disables home button, doesnt work?
    // appletSetFocusHandlingMode(AppletFocusHandlingMode_NoSuspend); // disables suspension, doesnt work?
    appletSetAutoSleepDisabled(true); // disables sleep

    if (UTIL::checkAmiiboDatabase() == true) { 
        
        std::ifstream i("sdmc:/emuiibo/amiibos.json");
        i >> amiibodata;

        AmiiboMenu menu(amiibodata);
        menu.mainLoop(); 
        
    }

    appletSetAutoSleepDisabled(false);
    // appletSetFocusHandlingMode(AppletFocusHandlingMode_SuspendHomeSleep);
    // appletUnlockExit();

    socketExit();
    appletExit();
    // setsysExit();
    consoleExit(NULL);
    return 0;
}