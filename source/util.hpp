#pragma once

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <filesystem>
#include <stdio.h>
#include <string>
#include <iostream>

#include <switch.h>
#include <curl/curl.h>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

namespace UTIL
{
    static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    static int downloadFile(std::string url, std::string path)
    {
        
        CURL *curl;
        FILE *fp;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            fp = fopen(path.c_str(), "w");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, UTIL::write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
            return res;
        }
        
        return -1;
    }
    
    static bool downloadAmiiboDatabase()
    {
        if (std::filesystem::exists("sdmc:/emuiibo/amiibos.json"))
        {
            remove("sdmc:/emuiibo/amiibos.json");
        }

        int ret = UTIL::downloadFile("https://www.amiiboapi.com/api/amiibo/", "sdmc:/emuiibo/amiibos.json");
        if (ret == 0)
        {
            return true;
        }
        return false;
    }

    static bool checkAmiiboDatabase()
    {
        if (!std::filesystem::exists("sdmc:/emuiibo/amiibos.json"))
        {
            printf("Amiibo database not present. Updating ...\n");
            bool res = UTIL::downloadAmiiboDatabase();
            if (!res)
            {
                printf("Failed to download amiibo database.\n");
                printf("Make sure you have an internet connection and an unmodified PRODINFO\n");
                printf("Alternatively, you can download the database manually from:\n");
                printf("https://www.amiiboapi.com/api/amiibo/");
                printf("\nand place it into:\n");
                printf("sdmc:/emuiibo/amiibos.json");
                printf("\n\n");
                printf("Press + to exit.\n");

                int loop = 15;
                while (loop)
                {
                    printf("Exiting in %d seconds...\n", loop);
                    consoleUpdate(NULL);
                    svcSleepThread(1000000000ull);
                    if (loop < 0){ break; }
                    loop -= 1;
                }
                return false; // did not download
            }
            return true; // downloaded
        }
        return true; // already present
    }

    static bool isBlacklistedCharacter(char c)
    {

        if (!(c >= 0 && c < 128))
        {
            return true;
        }
        switch (c)
        {
        case '!':
        case '?':
        case '.':
        case ',':
        case '\'':
        case '\\':
            return true;
        default:
            return false;
        }
    }

    static int RandU(int nMin, int nMax)
    {
        return nMin + (int)((double)rand() / ((double)RAND_MAX + 1) * (nMax - nMin + 1));
    }

    static uint16_t swap_uint16(uint16_t val)
    {
        return (val << 8) | (val >> 8);
    }

    static int loadAndResizeImageInRatio(std::string imagePath)
    {
        // load png image
        int width, height, channels;
        unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
        if (data == NULL)
        {
            printf("Error: failed to load image %s", imagePath.c_str());
            return 0;
        }

        // resize image to a height of 150px and keep the aspect ratio
        int newWidth = 150 * width / height;
        unsigned char *resizedData = (unsigned char *)malloc(newWidth * 150 * channels);
        stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, 150, 0, channels);

        // convert to RGBA if needed
        if (channels == 3)
        {
            unsigned char *rgbaData = (unsigned char *)malloc(newWidth * 150 * 4);
            for (int i = 0; i < newWidth * 150; i++)
            {
                rgbaData[i * 4 + 0] = resizedData[i * 3 + 0];
                rgbaData[i * 4 + 1] = resizedData[i * 3 + 1];
                rgbaData[i * 4 + 2] = resizedData[i * 3 + 2];
                rgbaData[i * 4 + 3] = 255;
            }
            free(resizedData);
            resizedData = rgbaData;
            channels = 4;
        }

        // save resized image
        stbi_write_png(imagePath.c_str(), newWidth, 150, channels, resizedData, newWidth * channels);

        // free memory
        free(data);
        free(resizedData);

        return 1;
    }
    
    /*
    static void delete_folder_contents(const std::string &path)
    {
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(path.c_str())) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
                {
                    std::string file = path + "/" + ent->d_name;
                    remove(file.c_str());
                }
            }
            closedir(dir);
        }
        remove(path.c_str());
        printf(".");
        consoleUpdate(NULL);
    }*/

}