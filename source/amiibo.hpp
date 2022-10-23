#ifndef INCLUDE_AMIIBO_HPP_
#define INCLUDE_AMIIBO_HPP_

#include <stdint.h>
#include <time.h>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>

#include <switch.h>
#include "json.hpp"
#include "util.hpp"

using json = nlohmann::json;

class Amiibo
{
public:
    json amiiboJsonDatabase;

    Amiibo()
    {
        srand(time(NULL)); // Initialization, should only be called once.
        std::ifstream ifs("sdmc:/emuiibo/amiibos.json");
        amiiboJsonDatabase = json::parse(ifs);
    }

    bool generateAmiibo(int number, int maxAmiibos)
    {

        json amiibo = amiiboJsonDatabase["amiibo"][number];

        time_t unixTime = time(NULL);
        struct tm *timeStruct = gmtime((const time_t *)&unixTime);
        int day = timeStruct->tm_mday;
        int month = timeStruct->tm_mon;
        int year = timeStruct->tm_year + 1900;

        std::string amiiboId = amiibo["head"].get<std::string>().append(amiibo["tail"].get<std::string>());
        // if length of amiiboId is less than 16, return false
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
        int character_game_id_int_swap = Util::swap_uint16(character_game_id_be);
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
        amiiboData["uuid"][0] = Util::RandU(0, 255);
        amiiboData["uuid"][1] = Util::RandU(0, 255);
        amiiboData["uuid"][2] = Util::RandU(0, 255);
        amiiboData["uuid"][3] = Util::RandU(0, 255);
        amiiboData["uuid"][4] = Util::RandU(0, 255);
        amiiboData["uuid"][5] = Util::RandU(0, 255);
        amiiboData["uuid"][6] = Util::RandU(0, 255);
        amiiboData["uuid"][7] = 0;
        amiiboData["uuid"][8] = 0;
        amiiboData["uuid"][9] = 0;

        std::string amiiboSeries = amiibo["amiiboSeries"].get<std::string>();
        std::string amiiboName = amiibo["name"].get<std::string>();

        amiiboSeries.erase(std::remove_if(amiiboSeries.begin(), amiiboSeries.end(), &Util::isBlacklistedCharacter), amiiboSeries.end());
        amiiboName.erase(std::remove_if(amiiboName.begin(), amiiboName.end(), &Util::isBlacklistedCharacter), amiiboName.end());

        std::erase_if(amiiboSeries, [](char16_t ch)
                      { return (ch >= 0xd800) && (ch <= 0xdfff); });
        std::erase_if(amiiboName, [](char16_t ch)
                      { return (ch >= 0xd800) && (ch <= 0xdfff); });

        // some amiibos have the same name, adding the number infront of the name ensures no overwrites.
        std::string amiiboPathFull = "sdmc:/emuiibo/amiibo/" + amiiboSeries + "/" + std::to_string(number) + "_" + amiiboName + "/";

        if (Util::check_folder_exist(amiiboPathFull.c_str()))
        {
            Util::delete_folder_with_content(amiiboPathFull.c_str());
        }

        Util::create_directory(amiiboPathFull.c_str());

        std::ofstream output(amiiboPathFull + "amiibo.flag");
        output.close();

        std::ofstream output2(amiiboPathFull + "amiibo.json");
        output2 << amiiboData.dump(2);
        output2.close();

        std::string text = "[" + std::to_string(number) + "/" + std::to_string(maxAmiibos) + "] generated: " + amiiboSeries + " - " + amiiboName + "\n";
        printf(text.c_str());
        consoleUpdate(NULL);
        return true;
    }

    bool generateAllAmibos()
    {
        for (long unsigned int i = 0; i < amiiboJsonDatabase["amiibo"].size(); i++)
        {
            generateAmiibo(i, amiiboJsonDatabase["amiibo"].size() - 1);
        }

        printf("\nAmiibos generated, you can exit now. (+)\n");
        consoleUpdate(NULL);

        return true;
    }

private:
};

#endif // INCLUDE_AMIIBO_HPP_