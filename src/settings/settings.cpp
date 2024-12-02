#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include "settings.hpp"
#include "../common/common.hpp"

std::string defaults = 
    "host=auto\n"
    "port=8080\n"
    "\n"
    "$white=FFFFFFFF\n"
    "$dark_blue=222255FF\n"
    "\n"
    "# colors are encoded in RGBA format #\n"
    "sendbgcolor=$white\n"
    "recvbgcolor=$white\n"
    "textcolor=$dark_blue\n"
    "qr_dark_color=$dark_blue\n"
    "qr_light_color=$white\n"
    "\n"
    "save_to=saved/\n"
;

void createDefaultSettings(std::string fname)
{
    std::ofstream sett_file(fname);

    if (!sett_file.is_open())
    {
        std::cerr << "Unable to open file " + fname + ". An attempt to create this file also failed\n";
        exit(0);
    }

    sett_file.write(defaults.c_str(), defaults.size());
    sett_file.close();
}

void parseSettingsFile(std::istream &f, AXONSETTINGSCONF &settings)
{
    std::map<std::string, std::string> cust_colors;

    std::string line;
    std::getline(f, line);
    do
    {
        trimWhitespace(line);

        std::string::size_type eq = line.find('=');
        if (eq != std::string::npos)
        {
            std::string field = line.substr(0, eq);
            std::string val = line.substr(eq+1, line.size()-1);
            //process val
            if (val[0] == '$')
            {
                val.erase(0, 1);
                std::map<std::string, std::string>::iterator ccol;    
                if ((ccol = cust_colors.find(val)) != cust_colors.end())
                {
                    val = ccol->second;
                }
            }

            //assign val
            if (field == "port")
                settings.port = std::stoi(val);
            if (field == "host")
                settings.host = val;
            if (field == "sendbgcolor")
                settings.sendbgcolor = std::stoul(val, nullptr, 16);
            if (field == "recvbgcolor")
                settings.recvbgcolor = std::stoul(val, nullptr, 16);
            if (field == "textcolor")
                settings.textcolor = std::stoul(val, nullptr, 16);
            if (field == "qr_dark_color")
                settings.qr_dark_color = std::stoul(val, nullptr, 16);
            if (field == "qr_light_color")
                settings.qr_light_color = std::stoul(val, nullptr, 16);
            if (field == "save_to")
            {
                if (std::string("\\/").find(val.back()) == std::string::npos)
                    val.push_back('/');
                settings.save_to = val;
            }
            if (field[0] == '$')
            {
                // trim leading $ from field
                std::string tfield = field.substr(1, field.size()-1);

                std::cout << "Before: (" << tfield << ',' << val << ")\n";

                // trim whitespace from field and val
                trimWhitespace(tfield);
                trimWhitespace(val);

                std::cout << "After: (" << tfield << ',' << val << ")\n";

                cust_colors.insert(std::pair(tfield, val));
            }
        }
    } while (std::getline(f, line));

    // Do value processing here
    if (settings.host == "auto")
        settings.host = GetLANIPAddress();
}

AXONSETTINGSCONF loadSettings(unsigned int argc, char** argv, std::string fname)
{
    AXONSETTINGSCONF settings;

    //load c-style args into c++ format
    std::vector<std::string> args;
    std::cout << "args:";
    for (int arg = 0; arg < argc; arg++)
    {
        args.push_back(argv[arg]);
        std::cout << "[" << argv[arg] << "] ";
    }
    std::cout << '\n';

    settings.url_scrambling = true;
    for (std::string arg : args)
    {
        if (arg == "/DISABLEURLSCRAMBLING")
            settings.url_scrambling = false;
    }

    std::ifstream sett_file(fname);
    if (!sett_file.is_open())
    {
        createDefaultSettings(fname);
        sett_file.open(fname);
        if (!sett_file.is_open())
        {
            std::cerr << "Unable to open file " + fname;
            exit(0);
        }
    }

    std::stringstream def_set_ss(defaults);
    parseSettingsFile(def_set_ss, settings);
    parseSettingsFile(sett_file, settings);

    return settings;
}