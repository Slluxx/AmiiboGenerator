#ifndef INCLUDE_UTIL_HPP_
#define INCLUDE_UTIL_HPP_

#include <curl/curl.h>
#include <filesystem>
#include <stdlib.h>
#include <string.h>

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
};

#endif // INCLUDE_UTIL_HPP_