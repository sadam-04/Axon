#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include "nayuki-qr/qrcodegen.hpp"
#include "webserver.hpp"
#include "dataloader.hpp"
#include "qr_to_bmp.hpp"
#include "window.hpp"
#include "gethost.hpp"

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

char QR_1_STR[2] = {(char)219, (char)219};
char QR_0_STR[2] = {(char)32, (char)32};

#define QR_FALSE

std::string getParentDirectory(const std::string& fullPath) {
    size_t lastSlashPos = fullPath.find_last_of("\\/");
    if (lastSlashPos != std::string::npos) {
        return fullPath.substr(0, lastSlashPos + 1); // Include trailing slash
    }
    return "";
}

int main(int argc, char* argv[]) {

    std::string fname;

    if (argc == 2)
    {
        fname = argv[1];
    }
    else
    {
        std::cin >> fname;

        if (fname.empty())
        {
            std::cerr << "Invalid filename\n";
            exit(1);
        }
    }

    std::string data = loadFile(fname);

    HOST host = getHost(getParentDirectory(argv[0]) + "host.txt");


    std::thread ws_thread(webserver, fname, data, host.second);

    std::string url = "http://" + host.first + ":" + std::to_string(host.second) + "/file";

    // Create a QR Code object
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeText(url.c_str(), qrcodegen::QrCode::Ecc::LOW);

    std::string bmpdata = makebmp(qr);

    std::thread sfml_thread(showBMP, bmpdata);

    sfml_thread.join();

    return 0;
}