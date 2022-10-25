
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>
#include "amiibo.hpp"
#include "util.hpp"

void print_menu()
{
    printf("Press + to exit.\n");
    printf("Press - to delete all amiibo.\n");
    printf("Press Y to download/update amiibo database (needs internet).\n");
    printf("Press X to generate all amiibos *.\n");
    printf("Press A to generate all amiibos with images * (very slow, needs internet).\n\n");
    printf("* Generating amiibos will first delete all existing ones for compability.\n\n");
    consoleUpdate(NULL);
}

void clickedGenerateAmiibo(bool dbExists, bool withImage = false)
{
    consoleClear();

    if (!dbExists)
    {
        printf("Amiibo database not found. Please download it first (Y).\n\n");
    }
    else
    {
        printf("Generating all amiibos...\n");
        Amiibo amiibo = Amiibo();
        amiibo.generateAllAmibos(withImage);
    }
}

void deleteAmiibos()
{
    consoleClear();
    if (Util::check_folder_exist("sdmc:/emuiibo/amiibo/"))
    {
        printf("Deleting all amiibos. This can take a few seconds.\n\n");
        consoleUpdate(NULL);
        Util::delete_folder_with_content("sdmc:/emuiibo/amiibo");
    }
    consoleClear();
    printf("All amiibo deleted.\n\n");
}

void downloadDatabase(bool amiiboFileExists)
{
    if (amiiboFileExists)
    {
        remove("sdmc:/emuiibo/amiibos.json");
    }
    printf("Downloading database...\n");
    consoleUpdate(NULL);
    int ret = Util::download_file("https://www.amiiboapi.com/api/amiibo/", "sdmc:/emuiibo/amiibos.json");
    consoleClear();
    if (ret == 0)
    {
        amiiboFileExists = true;
        printf("Database downloaded. You can now generate amiibos.\n\n");
    }
    else
    {
        printf("Failed to download database. Error code: %d\n", ret);
        printf("Make sure you have an internet connection and try again.\n\n");
    }
}

int main(int argc, char *argv[])
{
    consoleInit(NULL);
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);

    socketInitializeDefault();

    Util::create_directory("sdmc:/emuiibo/");
    bool amiiboFileExists = Util::check_file_exist("sdmc:/emuiibo/amiibos.json");
    if (!amiiboFileExists)
    {
        printf("Amiibo database not found.\n\n");
    }
    else
    {
        printf("Amiibo database found.\n\n");
    }

    print_menu();

    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;

        // I dont know why but this doesnt work for some reason. Crashes the application.
        if (kDown & HidNpadButton_Minus)
        {
            deleteAmiibos();
            print_menu();
        }

        if (kDown & HidNpadButton_Y)
        {
            downloadDatabase(amiiboFileExists);
            print_menu();
        }

        if (kDown & HidNpadButton_X)
        {
            deleteAmiibos();
            clickedGenerateAmiibo(amiiboFileExists, false);
            print_menu();
        }

        if (kDown & HidNpadButton_A)
        {
            deleteAmiibos();
            clickedGenerateAmiibo(amiiboFileExists, true);
            print_menu();
        }
        consoleUpdate(NULL);
    }

    socketExit();
    consoleExit(NULL);
    return 0;
}
