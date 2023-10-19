#pragma once

#include <fstream>
#include <iostream>
#include <vector>
#include <dirent.h>
#include <stdlib.h>
#include <algorithm>
#include <filesystem>

#include "UTIL.hpp"
#include "json.hpp"

using json = nlohmann::json;

class Amiibo
{
public:
    json amiibo;

    Amiibo(json data)
    {
        srand(time(NULL));
        amiibo = data;
    };

    bool generate(bool withImage = false)
    {
        time_t unixTime = time(NULL);
        struct tm *timeStruct = gmtime((const time_t *)&unixTime);
        int day = timeStruct->tm_mday;
        int month = timeStruct->tm_mon;
        int year = timeStruct->tm_year + 1900;

        std::string amiiboId = amiibo["head"].get<std::string>().append(amiibo["tail"].get<std::string>());
        if (amiiboId.length() < 16)
        {
            printf("Amiibo ID is invalid\n");
            return false;
        }

        std::string character_game_id_str = amiiboId.substr(0, 4);
        std::string character_variant_str = amiiboId.substr(4, 2);
        std::string character_figure_type_str = amiiboId.substr(6, 2);
        std::string model_no_str = amiiboId.substr(8, 4);
        std::string series_str = amiiboId.substr(12, 2);

        int character_game_id_be = (int)strtol(character_game_id_str.c_str(), nullptr, 16);
        int character_game_id_int_swap = UTIL::swap_uint16(character_game_id_be);
        int character_variant_be = (int)strtol(character_variant_str.c_str(), nullptr, 16);
        int figure_type_be = (int)strtol(character_figure_type_str.c_str(), nullptr, 16);
        int modelNumber_be = (int)strtol(model_no_str.c_str(), nullptr, 16);
        int series_be = (int)strtol(series_str.c_str(), nullptr, 16);

        json amiiboData = json::object();
        amiiboData["name"] = amiibo["name"];
        amiiboData["write_counter"] = 0;
        amiiboData["version"] = 0;

        amiiboData["first_write_date"] = json::object();
        amiiboData["first_write_date"]["y"] = year;
        amiiboData["first_write_date"]["m"] = month + 1;
        amiiboData["first_write_date"]["d"] = day;

        amiiboData["last_write_date"] = json::object();
        amiiboData["last_write_date"]["y"] = year;
        amiiboData["last_write_date"]["m"] = month + 1;
        amiiboData["last_write_date"]["d"] = day;

        amiiboData["mii_charinfo_file"] = "mii-charinfo.bin";

        amiiboData["id"] = json::object();
        amiiboData["id"]["game_character_id"] = character_game_id_int_swap;
        amiiboData["id"]["character_variant"] = character_variant_be;
        amiiboData["id"]["figure_type"] = figure_type_be;
        amiiboData["id"]["series"] = series_be;
        amiiboData["id"]["model_number"] = modelNumber_be;

        amiiboData["uuid"] = json::array();
        amiiboData["uuid"][0] = UTIL::RandU(0, 255);
        amiiboData["uuid"][1] = UTIL::RandU(0, 255);
        amiiboData["uuid"][2] = UTIL::RandU(0, 255);
        amiiboData["uuid"][3] = UTIL::RandU(0, 255);
        amiiboData["uuid"][4] = UTIL::RandU(0, 255);
        amiiboData["uuid"][5] = UTIL::RandU(0, 255);
        amiiboData["uuid"][6] = UTIL::RandU(0, 255);
        amiiboData["uuid"][7] = 0;
        amiiboData["uuid"][8] = 0;
        amiiboData["uuid"][9] = 0;

        std::string amiiboSeries = amiibo["amiiboSeries"].get<std::string>();
        std::string amiiboName = amiibo["name"].get<std::string>();

        amiiboSeries.erase(std::remove_if(amiiboSeries.begin(), amiiboSeries.end(), &UTIL::isBlacklistedCharacter), amiiboSeries.end());
        amiiboName.erase(std::remove_if(amiiboName.begin(), amiiboName.end(), &UTIL::isBlacklistedCharacter), amiiboName.end());

        /* i have no idea what i did here. i think the top code is the same as below?
        std::erase_if(amiiboSeries, [](char16_t ch)
                      { return (ch >= 0xd800) && (ch <= 0xdfff); });
        std::erase_if(amiiboName, [](char16_t ch)
                      { return (ch >= 0xd800) && (ch <= 0xdfff); });
        */

        // replace all spaces with underscores
        std::replace(amiiboSeries.begin(), amiiboSeries.end(), '/', '_');
        std::replace(amiiboName.begin(), amiiboName.end(), '/', '_');

        // some amiibos have the same name, adding the number suffix of the name ensures no overwrites.
        std::string amiiboPathFull = "sdmc:/emuiibo/amiibo/" + amiiboSeries + "/" + amiiboName + "_" + amiiboId + "/"; // "sdmc:/emuiibo/amiibo/" +

        // no need to create the amiibo if it already exists - delete it if you want to regenerate it.
        if (std::filesystem::exists(amiiboPathFull))
        {
            printf("Amiibo already exists.\n");
            return false;
        }

        // if it doesnt, create it.
        std::filesystem::create_directories(amiiboPathFull);
        if (std::filesystem::exists(amiiboPathFull))
        {
            std::ofstream output(amiiboPathFull + "amiibo.flag");
            output.close();

            std::ofstream output2(amiiboPathFull + "amiibo.json");
            output2 << amiiboData.dump(2);
            output2.close();

            if (withImage)
            {
                int ret = UTIL::downloadFile(amiibo["image"].get<std::string>(), amiiboPathFull + "amiibo.png");
                if (ret != 0)
                {
                    printf("Failed to download image. Error code: %d\n", ret);
                }
                else
                {
                    UTIL::loadAndResizeImageInRatio(amiiboPathFull + "amiibo.png");
                }
            }

        }
        return true;
    };

    bool erase(){
        std::string amiiboId = amiibo["head"].get<std::string>().append(amiibo["tail"].get<std::string>());
        if (amiiboId.length() < 16)
        {
            printf("Amiibo ID is invalid\n");
            return false;
        }

        std::string amiiboSeries = amiibo["amiiboSeries"].get<std::string>();
        std::string amiiboName = amiibo["name"].get<std::string>();

        amiiboSeries.erase(std::remove_if(amiiboSeries.begin(), amiiboSeries.end(), &UTIL::isBlacklistedCharacter), amiiboSeries.end());
        amiiboName.erase(std::remove_if(amiiboName.begin(), amiiboName.end(), &UTIL::isBlacklistedCharacter), amiiboName.end());

        std::replace(amiiboSeries.begin(), amiiboSeries.end(), '/', '_');
        std::replace(amiiboName.begin(), amiiboName.end(), '/', '_');

        std::string amiiboPathFull = "sdmc:/emuiibo/amiibo/" + amiiboSeries + "/" + amiiboName + "_" + amiiboId + "/";

        std::uintmax_t n = std::filesystem::remove_all(amiiboPathFull);

        return false;
    }
};