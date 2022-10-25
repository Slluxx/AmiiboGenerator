#ifndef INCLUDE_UTIL_HPP_
#define INCLUDE_UTIL_HPP_

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <curl/curl.h>
#include <filesystem>
#include <stdlib.h>
#include <string.h>

#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"

class Util
{
public:
    static bool create_directory(const std::string &path)
    {
        return std::filesystem::create_directories(path);
    }

    static bool check_file_exist(const std::string &path)
    {
        return std::filesystem::exists(path);
    }

    static bool check_folder_exist(const std::string &path)
    {
        return std::filesystem::exists(path);
    }

    static bool delete_folder_with_content(const std::string &path)
    {
        return std::filesystem::remove_all(path);
    }

    static int RandU(int nMin, int nMax)
    {
        return nMin + (int)((double)rand() / ((double)RAND_MAX + 1) * (nMax - nMin + 1));
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
            return true;
        default:
            return false;
        }
    }

    static uint16_t swap_uint16(uint16_t val)
    {
        return (val << 8) | (val >> 8);
    }

    static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
        size_t written = fwrite(ptr, size, nmemb, stream);
        return written;
    }

    static int download_file(const std::string &url, const std::string &path)
    {
        CURL *curl;
        FILE *fp;
        CURLcode res;
        curl = curl_easy_init();
        if (curl)
        {
            fp = fopen(path.c_str(), "w");
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Util::write_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            fclose(fp);
            return res;
        }
        return -1;
    }

    static int loadAndResizeImageInRatio(std::string imagePath){
        // load png image
        int width, height, channels;
        unsigned char *data = stbi_load(imagePath.c_str(), &width, &height, &channels, 0);
        if (data == NULL) {
            printf("Error: failed to load image %s", imagePath.c_str());
            return 0;
        }

        // resize image to a height of 150px and keep the aspect ratio
        int newWidth = 150 * width / height;
        unsigned char *resizedData = (unsigned char *)malloc(newWidth * 150 * channels);
        stbir_resize_uint8(data, width, height, 0, resizedData, newWidth, 150, 0, channels);

        // convert to RGBA if needed
        if (channels == 3) {
            unsigned char *rgbaData = (unsigned char *)malloc(newWidth * 150 * 4);
            for (int i = 0; i < newWidth * 150; i++) {
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
};

#endif // INCLUDE_UTIL_HPP_