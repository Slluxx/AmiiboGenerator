
#include <stdio.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <unistd.h>
#include "amiibo.hpp"
#include "util.hpp"

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
        printf("Amiibo database not found. Press Y to download it.\n\n");
    }
    else
    {
        printf("Amiibo database found.\nPress Y to update, X to generate all amiibos or + to exit.\n\n");
    }

    consoleUpdate(NULL);

    while (appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & HidNpadButton_Plus)
            break;

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
                printf("Database downloaded.\n");
                printf("You can now press X to generate all amiibos, Y to update again or + to exit.\n\n");
                amiiboFileExists = true;
            }
            else
            {
                printf("Failed to download database. Error code: %d\n", ret);
                printf("Make sure you have an internet connection and try again.\n\n");
            }
        }

        if (kDown & HidNpadButton_X)
        {
            if (!amiiboFileExists)
            {
                printf("Amiibo database not found. Please download it first.\n\n");
            }
            else
            {

                printf("Generating all amiibos...\n");
                Amiibo amiibo = Amiibo();
                amiibo.generateAllAmibos();
            }
        }
        consoleUpdate(NULL);
    }

    socketExit();
    consoleExit(NULL);
    return 0;
}
