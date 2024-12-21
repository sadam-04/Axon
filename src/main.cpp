#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <queue>
#include <mutex>
#include "common/common.hpp"
#include "nayuki-qr/qrcodegen.hpp"
#include "webserver.hpp"
#include "window.hpp"
#include "settings/settings.hpp"

#include <wx/wx.h>

char QR_1_STR[2] = {(char)219, (char)219};
char QR_0_STR[2] = {(char)32, (char)32};

std::string ROOT_DIR;
AXONSETTINGSCONF SETTINGS;

// int main(int argc, char* argv[]) {

class wxAxon : public wxApp {
public:
    virtual bool OnInit() override {
        ROOT_DIR = getExecutableDirectory(); //DOES NOT include trailing slash/
        SETTINGS = loadSettings(wxApp::argc, wxApp::argv, ROOT_DIR + "\\settings.txt");

        std::string data;
        
        // Load target data from file
        if (!SETTINGS.served_file.empty())
            data = loadFile(SETTINGS.served_file);

        // Build url
        std::string url_path_send = '/' + (SETTINGS.url_scrambling ? randAN(5) : "download");
        std::string url_path_recv = '/' + (SETTINGS.url_scrambling ? randAN(5) : "upload");

        while (SETTINGS.url_scrambling && url_path_recv == url_path_send)
            url_path_recv = '/' + (SETTINGS.url_scrambling ? randAN(5) : "file");

        std::string url_send = "http://" + SETTINGS.host + ":" + std::to_string(SETTINGS.port) + url_path_send;
        std::string url_recv = "http://" + SETTINGS.host + ":" + std::to_string(SETTINGS.port) + url_path_recv;

        // File candidate queue for RECV mode
        std::queue<FileRC> file_q;
        std::mutex file_q_mutex;

        // Start webserver
        std::thread ws_thread(webserver, SETTINGS.served_file, std::ref(data), SETTINGS.port, url_path_send, url_path_recv, std::ref(file_q), std::ref(file_q_mutex));

        // // Create QR
        // qrcodegen::QrCode qr_send = qrcodegen::QrCode::encodeText(url_send.c_str(), qrcodegen::QrCode::Ecc::LOW);
        // qrcodegen::QrCode qr_recv = qrcodegen::QrCode::encodeText(url_recv.c_str(), qrcodegen::QrCode::Ecc::LOW);
        // std::string qr_bmp_send = makebmp(qr_send, SETTINGS.qr_light_color >> 8, SETTINGS.qr_dark_color >> 8); // 0x ## RR GG BB
        // std::string qr_bmp_recv = makebmp(qr_recv, SETTINGS.qr_light_color >> 8, SETTINGS.qr_dark_color >> 8);




        // SFML (OLD) IMPLEMENTATION
        // while (1)
        // {
        //     //if there is a file being served
        //     if (!SETTINGS.served_file.empty())
        //     {
        //         //create a send window
        //         if (create_window(qr_bmp_send, url_send, file_q, file_q_mutex, getFilename(SETTINGS.served_file))) break;
        //     }
        //     // create a recv window
        //     if (create_window(qr_bmp_recv, url_recv, file_q, file_q_mutex)) break;
        // }

        wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "hello world");
        frame->Show();
        return true;
    }
};

wxIMPLEMENT_APP(wxAxon);





// class wxAxon : public wxApp {
// public:
//     virtual bool OnInit() override {
//         wxFrame* frame = new wxFrame(nullptr, wxID_ANY, "hello world");
//         frame->Show();
//         return true;
//     }
// };

// wxIMPLEMENT_APP(wxAxon);