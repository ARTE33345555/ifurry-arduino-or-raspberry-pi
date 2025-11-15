#include <SDL2/SDL.h>
#include <libcamera/libcamera.h>
#include <simpleble/SimpleBLE.h>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <cstdlib>

using namespace libcamera;
using namespace std::chrono_literals;

// ------------------ EyeManager ------------------
struct Eye {
    int x, y;
    int homeX, homeY;
};

class EyeManager {
public:
    void init() {
        SDL_Init(SDL_INIT_VIDEO);
        window = SDL_CreateWindow("FurDisplay Eyes", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 240, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        eye1 = {160, 120, 160, 120};
        eye2 = {160, 120, 160, 120};
    }

    void update(const std::vector<std::pair<int,int>>& coords) {
        if(coords.size() > 0) {
            eye1.x = coords[0].first;
            eye1.y = coords[0].second;
            if(coords.size() > 1){
                eye2.x = coords[1].first;
                eye2.y = coords[1].second;
            }
        } else {
            eye1.x = eye1.homeX; eye1.y = eye1.homeY;
            eye2.x = eye2.homeX; eye2.y = eye2.homeY;
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_Rect r1 = {eye1.x - 10, eye1.y - 10, 20, 20};
        SDL_Rect r2 = {eye2.x - 10, eye2.y - 10, 20, 20};
        SDL_RenderFillRect(renderer, &r1);
        SDL_RenderFillRect(renderer, &r2);

        SDL_RenderPresent(renderer);
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Eye eye1, eye2;
};

// ------------------ CameraManager ------------------
class CameraManager {
public:
    void init() {
        std::cout << "Initializing libcamera...\n";
        cm = std::make_unique<CameraManager>();
        cm->start();

        const std::vector<std::shared_ptr<Camera>>& cameras = cm->cameras();
        if (cameras.empty()) {
            std::cerr << "No cameras found!" << std::endl;
            return;
        }

        camera = cameras[0];
        camera->acquire();
    }

    void captureFrames() {
        // Заглушка для координат
        coords = { {rand()%320, rand()%240}, {rand()%320, rand()%240} };
    }

    std::vector<std::pair<int,int>> getObjectCoordinates() {
        return coords;
    }

private:
    std::unique_ptr<CameraManager> cm;
    std::shared_ptr<Camera> camera;
    std::vector<std::pair<int,int>> coords;
};

// ------------------ BluetoothManager ------------------
class BluetoothManager {
public:
    void init() {
        std::cout << "Initializing Bluetooth...\n";
        adapter = SimpleBLE::Adapter();
        adapter.ScanForPeripherals(5000); // Сканировать 5 секунд

        for (auto& peripheral : adapter.Peripherals()) {
            std::cout << "Found peripheral: " << peripheral.Name() << std::endl;
        }
    }

    void sendStatus(const std::vector<std::pair<int,int>>& coords, const std::string& battery) {
        std::cout << "Sending via BT: battery=" << battery << "\n";
    }

private:
    SimpleBLE::Adapter adapter;
};

// ------------------ BatteryManager ------------------
class BatteryManager {
public:
    void init() {}
    void update() {}
    std::string getStatus() { return "95%"; }
};

// ------------------ main ------------------
int main() {
    EyeManager eyes;
    CameraManager cameras;
    BluetoothManager bt;
    BatteryManager battery;

    eyes.init();
    cameras.init();
    bt.init();
    battery.init();

    // Потоки камер
    std::thread camThread([&]() {
        while(true) {
            cameras.captureFrames();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    });

    // Поток глаз
    std::thread eyeThread([&]() {
        while(true) {
            auto coords = cameras.getObjectCoordinates();
            eyes.update(coords);
            eyes.render();
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }
    });

    // Поток Bluetooth
    std::thread btThread([&]() {
        while(true) {
            auto coords = cameras.getObjectCoordinates();
            auto batt = battery.getStatus();
            bt.sendStatus(coords, batt);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });

    camThread.join();
    eyeThread.join();
    btThread.join();

    return 0;
}
