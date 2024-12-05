#include <SFML/Graphics.hpp>
#include <iostream>
#include <queue>
#include <mutex>
#include "common/common.hpp"
#include "icon/icon.hpp"
#include "settings/settings.hpp"

#pragma comment(lib, "sfml-graphics.lib")
#pragma comment(lib, "sfml-window.lib")
#pragma comment(lib, "sfml-system.lib")

extern AXONSETTINGSCONF SETTINGS;

const char HORIZ = 0b00000001;
const char VERT =  0b00000010;

void centerObj(sf::Transformable &obj, sf::FloatRect &bounds, sf::RenderWindow &w, const unsigned char axis)
{
    sf::Vector2u wSize = w.getSize();

    sf::Vector2f center = obj.getPosition();

    if (axis & HORIZ) center.x = (wSize.x - bounds.width) / 2;
    if (axis & VERT) center.y = (wSize.y - bounds.height) / 2;

    obj.setPosition(center);
}

extern std::string ROOT_DIR;

int create_window(std::string qr_bmp, std::string url, std::queue<FileRC> &file_q, std::mutex &file_q_mutex, std::string filename = "") {
    const unsigned int W_WIDTH_SEND = 375;
    const unsigned int W_WIDTH_RECV = 675;
    const unsigned int W_HEIGHT = 380;

    const unsigned int QR_MARGIN = 15;

    bool MODE_SEND = false;
    bool MODE_RECV = false;
    if (filename.empty())
        MODE_RECV = true;
    else
        MODE_SEND = true;

    sf::RenderWindow window(sf::VideoMode((MODE_SEND ? W_WIDTH_SEND : W_WIDTH_RECV), W_HEIGHT), "Axon", sf::Style::Close);
    //window.setFramerateLimit(30);
    window.setVerticalSyncEnabled(true);
    window.setIcon(ICON_WIDTH, ICON_HEIGHT, ICON_RGBA);

    // Load the BMP image into a texture
    sf::Texture qr_tex;

    if (!qr_tex.loadFromMemory(qr_bmp.c_str(), qr_bmp.size())) {
        std::cerr << "Error: Could not load recv qr BMP file!" << std::endl;
        return 1;
    }

    sf::Font font;
    if (!font.loadFromFile(ROOT_DIR + "/static/font/GeistMono-SemiBold.ttf"))
    {
        std::cerr << "Unable to load font\n";
        exit(0);
    }
    
    //Header and footer/url text
    sf::Text fn_text, url_text, recv_file;
    fn_text.setFont(font);
    url_text.setFont(font);
    recv_file.setFont(font);
    fn_text.setString((MODE_SEND ? filename : "Receiving"));
    url_text.setString(url);
    // set recv text later
    fn_text.setCharacterSize(22);
    url_text.setCharacterSize(16);
    recv_file.setCharacterSize(16);
    fn_text.setFillColor(sf::Color::White);
    url_text.setFillColor(sf::Color::White);
    recv_file.setFillColor(sf::Color::White);

    fn_text.setFillColor(sf::Color(SETTINGS.textcolor));
    url_text.setFillColor(sf::Color(SETTINGS.textcolor));
    recv_file.setFillColor(sf::Color(SETTINGS.textcolor));

    // QR Sprite
    sf::Sprite qr_spr;
    qr_spr.setTexture(qr_tex);
    
    sf::FloatRect qrbounds = qr_spr.getGlobalBounds();
    float qrScalar = 300.f/qrbounds.height;
    qr_spr.setScale(qrScalar, qrScalar);

    // Center elements
    centerObj(qr_spr, qr_spr.getGlobalBounds(), window, VERT | (HORIZ & MODE_SEND));
    if (MODE_RECV) qr_spr.move({QR_MARGIN, 0.f});
    centerObj(url_text, url_text.getGlobalBounds(), window, HORIZ);
    centerObj(fn_text, fn_text.getGlobalBounds(), window, HORIZ);

    recv_file.setPosition(W_WIDTH_RECV * 0.55f, W_HEIGHT * 0.1f);

    // Vertically shift header and footer text
    url_text.setPosition(url_text.getPosition().x, qr_spr.getGlobalBounds().top + qr_spr.getGlobalBounds().height + 10.f);
    fn_text.setPosition(fn_text.getPosition().x, qr_spr.getGlobalBounds().top - fn_text.getLocalBounds().height - 15.f);

    // file accept / discard buttons
    sf::RectangleShape accept_btn({40, 40}), discard_btn({40, 40});
    accept_btn.setFillColor(sf::Color::Green);
    discard_btn.setFillColor(sf::Color::Red);
    centerObj(accept_btn, accept_btn.getGlobalBounds(), window, HORIZ | VERT);
    accept_btn.move({50.f, 0.f});
    centerObj(discard_btn, discard_btn.getGlobalBounds(), window, HORIZ | VERT);
    discard_btn.move({150.f, 0.f});

    // file queue update timer
    sf::Clock clock;
    sf::Time interval = sf::milliseconds(500); //500ms
    sf::Time elapsedTime = interval; //init to interval so the first trigger occurs immediately

    bool queue_empty = true;

    while (window.isOpen()) {
        sf::Event event;
        elapsedTime += clock.restart();

        // Check if 500ms have passed
        if (elapsedTime >= interval) {
            
            elapsedTime -= interval; // Reset elapsed time, keep the remaining time for accuracy
            if (file_q.empty())
            {
                queue_empty = true;
                recv_file.setString("Uploads will appear here");
            }
            else
            {
                queue_empty = false;
                std::lock_guard<std::mutex> lock(file_q_mutex);
                recv_file.setString(file_q.front().getShortName() + "\n    (y) to accept\n    (n) to discard");
            }
        }
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
            {
                window.close();
                return 1;
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left)
            {
                // std::cout << "EVENT!!!!!!!!!\n";
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                if (accept_btn.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    if (!file_q.empty())
                    {
                        std::thread fileSave(pop_filerc, std::ref(file_q));
                        fileSave.detach();
                        elapsedTime = interval; // refresh file queue display
                    }
                }
                else if (discard_btn.getGlobalBounds().contains(mousePos.x, mousePos.y))
                {
                    if (!file_q.empty())
                    {
                        file_q.pop();
                        elapsedTime = interval; // refresh file queue display
                    }
                }
            }
            if (event.type == sf::Event::KeyPressed)
                switch (event.key.scancode)
                {
                    case sf::Keyboard::Scan::Escape:
                        window.close();
                        exit(0);
                    case sf::Keyboard::Scan::Space:
                        if (!SETTINGS.served_file.empty())
                        {
                            window.close();
                            return 0;
                        }
                        else
                            break;
                    case sf::Keyboard::Scan::Y:
                        if (!file_q.empty())
                        {
                            std::thread fileSave(pop_filerc, std::ref(file_q));
                            fileSave.detach();
                            elapsedTime = interval; // refresh file queue display
                        }
                        break;
                    case sf::Keyboard::Scan::N:
                        if (!file_q.empty())
                        {
                            file_q.pop();
                            elapsedTime = interval; // refresh file queue display
                        }
                }
        }

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        // std::cout << "x:" << mousePos.x << ", y:" << mousePos.y <<'\n';
        if (accept_btn.getGlobalBounds().contains(mousePos.x, mousePos.y))
        {
            // std::cout << "accept\n";
            accept_btn.setFillColor(sf::Color(0x33FF33FF));
        }
        else
        {
            accept_btn.setFillColor(sf::Color(0x00FF00FF));
        }

        if (discard_btn.getGlobalBounds().contains(mousePos.x, mousePos.y))
        {
            // std::cout << "discard\n";
            discard_btn.setFillColor(sf::Color(0xFF3333FF));
        }
        else
        {
            discard_btn.setFillColor(sf::Color(0xFF0000FF));
        }

        // Clear, draw, and display
        window.clear(sf::Color(MODE_SEND ? SETTINGS.sendbgcolor : SETTINGS.recvbgcolor));
        window.draw(fn_text);
        window.draw(url_text);
        if (MODE_RECV)
        {
            window.draw(recv_file);
            if (!queue_empty)
            {
                window.draw(accept_btn);
                window.draw(discard_btn);
                // std::cout << "BUTTONSSSSZZ\n";
            }
        }
        window.draw(qr_spr);

        window.display();
    }

    return 0;
}