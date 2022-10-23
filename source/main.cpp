
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>
#include "amiibo.hpp"
#include "util.hpp"

void print_menu()
{
    printf("Press + to exit.\n");
    // printf("Press - to delete all currently generated amiibo.\n");
    printf("Press Y to download/update amiibo database (needs internet).\n");
    printf("Press X to generate all amiibos.\n");
    printf("Press A to generate all amiibos with images (very slow, needs internet).\n\n");
    consoleUpdate(NULL);
}

void clickedGenerateAmiibo(bool dbExists, bool withImage = false)
{
    if (!dbExists)
    {
        printf("Amiibo database not found. Please download it first (Y).\n\n");
    }
    else
    {
        printf("Generating all amiibos...\n");
        Amiibo amiibo = Amiibo();
        amiibo.generateAllAmibos(withImage);
        print_menu();
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

        /*
        // I dont know why but this doesnt work for some reason. Crashes the application.
        if (kDown & HidNpadButton_Minus)
        {
            printf("Deleting all generated amiibos....\n");
            if (Util::check_folder_exist("sdmc:/emuiibo/amiibo/"))
            {
                Util::delete_folder_with_content("sdmc:/emuiibo/amiibo");
            }
            printf("All generated amiibo deleted.\n\n");
        }
        */

        if (kDown & HidNpadButton_Y)
        {
            if (amiiboFileExists)
            {
                remove("sdmc:/emuiibo/amiibos.json");
            }
            printf("Downloading database...\n");
            consoleUpdate(NULL);
            int ret = Util::download_file("https://www.amiiboapi.com/api/amiibo/", "sdmc:/emuiibo/amiibos.json");
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

        if (kDown & HidNpadButton_X)
        {
            clickedGenerateAmiibo(amiiboFileExists, false);
        }

        if (kDown & HidNpadButton_A)
        {
            clickedGenerateAmiibo(amiiboFileExists, true);
        }
        consoleUpdate(NULL);
    }

    socketExit();
    consoleExit(NULL);
    return 0;
}
