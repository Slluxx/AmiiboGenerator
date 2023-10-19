#pragma once
#include <switch.h>
#include <string>
#include <filesystem>

#include "json.hpp"
#include "amiibo.hpp"

using namespace std;
using json = nlohmann::json;

enum class AvailableScreens {
    MAIN,
    HELP,
    CURRENT,
};

class AmiiboMenu 
{
public:
    json amiibodata;

    const int itemsPerPage = 40;
    const char* info[4] = { "amiiboSeries", "character", "gameSeries", "name" };

    int currentSelectedAmiibos = 0;
    AvailableScreens currentScreen = AvailableScreens::MAIN;
    int currentMainPage = 1;
    int currenentMainCursorPos = 1;
    int currentInfoIndex = 0;

    bool generateWithImage = false;
    bool overwriteExisting = false;

    bool shallExit = false;

    PadState pad;
    
    AmiiboMenu(json AmiiboData){
        amiibodata = AmiiboData;
    };

    int getMaxPage(){
        return (amiibodata["amiibo"].size() + itemsPerPage - 1) / itemsPerPage;
    }

    void toggleAllAmiibo(){
        clearScreen();
        printf("Toggling all amiibos. This might take a few seconds.\n");
        consoleUpdate(NULL);

        int newselected = 0;
        for (int i = 0; i < (int)amiibodata["amiibo"].size(); i++) {
            json& currentItem = amiibodata["amiibo"][i];
            if (currentItem.contains("selected")) {
                bool selected = currentItem["selected"].get<bool>();
                if (selected == false) {
                    newselected++;
                }
                currentItem["selected"] = !selected;
            } else {
                currentItem["selected"] = true;
                newselected++;
            }
        }

        currentSelectedAmiibos = newselected;
        updateScreen(AvailableScreens::MAIN);
    }

    void updateAmiiboDatabase(){

        clearScreen();
        printf("Updating amiibo database. This might take a few seconds.\n");
        consoleUpdate(NULL);

        if (std::filesystem::exists("sdmc:/emuiibo/amiibos.json"))
        {
            remove("sdmc:/emuiibo/amiibos.json");
        }

        if (UTIL::checkAmiiboDatabase() == false){
            printf("Download failed!\n ");
            shallExit = true;
            return;
        }

        printf("Database updated!\n");
        consoleUpdate(NULL);
        std::ifstream i("sdmc:/emuiibo/amiibos.json");
        i >> amiibodata;
        
        int loop = 5;
        while (loop)
        {
            printf("Back in %d seconds...\n", loop);
            consoleUpdate(NULL);
            svcSleepThread(1000000000ull);
            if (loop < 0){ break; }
            loop -= 1;
        }

        updateScreen(AvailableScreens::MAIN);

    }

    void toggleImageGeneration(){
        generateWithImage = !generateWithImage;
        updateScreen(); 
    }

    void clearScreen() { consoleClear(); /* consoleUpdate(NULL); */ };

    void updateScreen(AvailableScreens screen = AvailableScreens::CURRENT) {
        clearScreen();

        if (screen != AvailableScreens::CURRENT) {
            if (currentScreen != screen) {
                currentScreen = screen;
            }
        }
        
        // failsafe
        if (currentScreen == AvailableScreens::CURRENT) {
            currentScreen = AvailableScreens::MAIN;
        }

        switch (currentScreen) {
            case AvailableScreens::MAIN:
                screen_main();
                break;
            case AvailableScreens::HELP:
                screen_help();
                break;
            default:
                screen_main();
                break;
        }

        consoleUpdate(NULL);
    }

    void screen_help() {
        printf("Amiibo Menu - Help & Controls\n\n");
        printf(
            "Image generation       : %s\n"
            "Current amiibo info    : %s\n\n"
            "Controls:\n\n"
            "+           : exit\n"
            "-           : update database\n"
            "Dpad UP     : cursor UP\n"
            "Dpad DOWN   : cursor DOWN\n"
            "Dpad LEFT   : page -1\n"
            "Dpad RIGHT  : page +1\n"
            "L           : page -10\n"
            "R           : page +10\n"
            "ZL          : toggle select all\n"
            "ZR          : toggle image generation (REQUIRES INTERNET)\n"
            "A           : toggle selection\n"
            "B           : toggle info screen\n"
            "X           : generate selected amiibo\n"
            "Y           : cycle amiibo info\n"
            "LS          : delete current amiiboSeries\n\n"
            
            "Press B to go back.\n", generateWithImage ? "Enabled" : "Disabled", info[currentInfoIndex]
        );
    }

    void screen_main() {
        showTopInfo();
        showPage(currentMainPage);
    }

    void nextInfoIndex(){
        int maxLength = sizeof(info) / sizeof(info[0]);
        currentInfoIndex += 1;
        if (currentInfoIndex >= maxLength) {
            currentInfoIndex = 0;
        }
        updateScreen();
    }

    void showTopInfo(){
        printf("Amiibo Menu - selected %d/%lu - Page %d/%d\n", currentSelectedAmiibos, amiibodata["amiibo"].size(), currentMainPage, getMaxPage());
        printf("Press B to toggle the info & controls screen.\n\n");
        // printf("A: Select | L/R: Page -/+ 10 | ZL/ZR: Page -/+ 100 | Y: Toggle info (%s)\n", info[currentInfoIndex]);
        // printf("-: Update database | +: Exit | X: Generate Amiibo\n\n");
    }

    /* modify related to MAIN screen*/

    void showPage(int page){
        // Calculate the starting and ending indices for the current page
        int startIndex = (page - 1) * itemsPerPage;
        int endIndex = min(startIndex + itemsPerPage, static_cast<int>(amiibodata["amiibo"].size()));

        if (startIndex >= (int)amiibodata["amiibo"].size()) {
            printf("Page number is out of range.\n");
            return;
        }

        // Extract the subset of data for the current page
        int localIndex = 0;
        for (int i = startIndex; i < endIndex; i++) {
            localIndex++;
            const json& currentItem = amiibodata["amiibo"][i];
            // Process currentItem as needed
            // cout << "Item " << i + 1 << ": " << currentItem << endl;
            showItem(localIndex, i+1, currentItem);
        }
    }

    void showItem(int localIndex, int gloabalIndex, json data ){
        string indicatorSelected = "N";
        if (data.contains("selected")) {
            if (data["selected"].get<bool>()) {
                indicatorSelected = "Y";
            }
        }
        string cursor = " ";
        if (localIndex == currenentMainCursorPos) {
            cursor = ">";
        }
        string text = data[info[currentInfoIndex]].get<string>();
        printf("%s [%s] %d) %s\n", cursor.c_str(), indicatorSelected.c_str(), gloabalIndex, text.c_str());
    }

    void changePage(int delta){
        int newPage = currentMainPage + delta;
        if (newPage < 1) { newPage = 1; }
        if (newPage > getMaxPage()) { newPage = getMaxPage(); }
        currentMainPage = newPage;
        updateScreen();
    }

    void changeCursorPosition(int delta){
        int newCursorPosition = currenentMainCursorPos + delta;
        if (newCursorPosition < 1) { newCursorPosition = itemsPerPage; }
        if (newCursorPosition > itemsPerPage) { newCursorPosition = 1; }
        currenentMainCursorPos = newCursorPosition;
        updateScreen();
    }

    void toggleCurrentItem(){
        int index = (currentMainPage - 1) * itemsPerPage + currenentMainCursorPos - 1;
        if (index >= (int)amiibodata["amiibo"].size()) {
            printf("Item number is out of range.\n");
            return;
        }

        json& currentItem = amiibodata["amiibo"][index];
        if (currentItem.contains("selected")) {
            currentItem["selected"] = !currentItem["selected"].get<bool>();
        } else {
            currentItem["selected"] = true;
        }

        if (currentItem["selected"].get<bool>()) {
            currentSelectedAmiibos++;
        } else {
            currentSelectedAmiibos--;
        }

        updateScreen();
    }

    void inputHandler(){
        int _null = 0;
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (currentScreen == AvailableScreens::MAIN) {
            if (kDown & HidNpadButton_Plus) // works
                shallExit = true;
            if (kDown & HidNpadButton_Minus) // works
                updateAmiiboDatabase();
            if (kDown & HidNpadButton_Up) // works
                changeCursorPosition(-1);
            if (kDown & HidNpadButton_Down) // works
                changeCursorPosition(+1);
            if (kDown & HidNpadButton_Left) // works
                changePage(-1);
            if (kDown & HidNpadButton_Right) // works
                changePage(+1);
            if (kDown & HidNpadButton_L) // works
                changePage(-10);
            if (kDown & HidNpadButton_R) // works
                changePage(+10);
            if (kDown & HidNpadButton_ZL)
                toggleAllAmiibo();
            if (kDown & HidNpadButton_ZR) // works
                toggleImageGeneration();
            if (kDown & HidNpadButton_A) // works
                toggleCurrentItem();
            if (kDown & HidNpadButton_B) // works
                updateScreen(AvailableScreens::HELP);
            if (kDown & HidNpadButton_X) // works
                generateAmiibo();
            if (kDown & HidNpadButton_Y) // works
                nextInfoIndex();
            if (kDown & HidNpadButton_StickL) // delete selected amiibo
                deleteSelectedAmiibo();
            
        } else if (currentScreen == AvailableScreens::HELP) {
            if (kDown & HidNpadButton_ZR) // works
                toggleImageGeneration();
            if (kDown & HidNpadButton_Y) // works
                nextInfoIndex();
            if (kDown & HidNpadButton_B) // works
                updateScreen(AvailableScreens::MAIN);
            if (kDown & HidNpadButton_Plus) // works
                shallExit = true;
        }
    }

    void generateAmiibo(){
        clearScreen();

        int generatedAmiibo = 1;

        for (int i = 0; i < (int)amiibodata["amiibo"].size(); i++) {
            const json& currentItem = amiibodata["amiibo"][i];
            if (currentItem.contains("selected")) {
                if (currentItem["selected"].get<bool>()) {
                    printf("%d/%d - Generating: %s\n", generatedAmiibo, currentSelectedAmiibos, currentItem["name"].get<string>().c_str());
                    consoleUpdate(NULL);
                    Amiibo amiibo(currentItem);
                    amiibo.generate(generateWithImage);
                    generatedAmiibo++;
                }
            }
        }
        printf("Done!\n");
        printf("Press B to go back.\n");
        consoleUpdate(NULL);

        bool loop = true;
        while (loop){
            padUpdate(&pad);
            u64 kDown = padGetButtonsDown(&pad);

            if (kDown & HidNpadButton_B)
                loop = false;
            svcSleepThread(50000000ull); // .05
        }

        updateScreen(AvailableScreens::MAIN);
    }

    void deleteSelectedAmiibo(){
        clearScreen();
        printf("Deleting amiibo. This might take a few seconds.\n");
        consoleUpdate(NULL);

        for (int i = 0; i < (int)amiibodata["amiibo"].size(); i++) {
            const json& currentItem = amiibodata["amiibo"][i];
            if (currentItem.contains("selected")) {
                if (currentItem["selected"].get<bool>()) {
                    printf("Deleting: %s\n", currentItem["name"].get<string>().c_str());
                    consoleUpdate(NULL);
                    Amiibo amiibo(currentItem);
                    amiibo.erase();
                    printf("Done!\n");
                    consoleUpdate(NULL);
                }
            }
        }

        updateScreen(AvailableScreens::MAIN);
    }

    int mainLoop(){

        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&pad);

        updateScreen();

        while (appletMainLoop()){
            if (shallExit) {
                return -1;
            }
            inputHandler();
            svcSleepThread(50000000ull); // .05
        }
        return 0;
    }

};