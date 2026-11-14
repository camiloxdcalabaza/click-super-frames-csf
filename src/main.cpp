#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/CCParticleSystem.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <mutex>
#include <deque>
#include <chrono>
#include <filesystem>

using namespace geode::prelude;

#ifdef GEODE_IS_WINDOWS
#include <windows.h>
#endif

namespace CFS {
    int g_realHz = 60;
    bool g_initialized = false;
    float g_currentFPS = 60.0f;
    int g_frameCount = 0;
    double g_lastFPSTime = 0;
    double g_lastClickTime = 0;
    bool g_lastClickDown = false;
    int g_nextInputId = 0;
    int g_inputsDropped = 0;
    int g_totalInputsProcessed = 0;
    float g_prevPlayerX = 0;
    float g_prevPlayerY = 0;
    float g_lastPlayerX = 0;
    float g_lastPlayerY = 0;
    float g_extrapolatedX = 0;
    float g_extrapolatedY = 0;
    int g_particleBudget = 150;
    int g_stuckFrames = 0;

    struct SuperInput {
        double timestamp;
        bool down;
        int key;
        bool processed;
        int id;
    };

    std::deque<SuperInput> g_inputQueue;
    std::mutex g_inputMutex;

    int detectRealHz() {
    #ifdef GEODE_IS_WINDOWS
        DEVMODEA dm{};
        dm.dmSize = sizeof(dm);
        if (EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
            if (dm.dmDisplayFrequency >= 50 && dm.dmDisplayFrequency <= 500) {
                return dm.dmDisplayFrequency;
            }
        }
    #endif
        return 60;
    }

    void updateFPSCounter() {
        double now = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        g_frameCount++;
        if (now - g_lastFPSTime >= 0.5) {
            g_currentFPS = static_cast<float>(g_frameCount / (now - g_lastFPSTime));
            g_frameCount = 0;
            g_lastFPSTime = now;
            if (g_currentFPS < 50) {
                g_particleBudget = 50;
            } else if (g_currentFPS < 100) {
                g_particleBudget = 100;
            } else {
                g_particleBudget = 150;
            }
        }
    }

    void processInputQueue() {
        std::lock_guard<std::mutex> lock(g_inputMutex);
        double now = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        int processedThisFrame = 0;
        while (!g_inputQueue.empty() && processedThisFrame < 10) {
            SuperInput input = g_inputQueue.front();
            g_inputQueue.pop_front();
            if (input.processed) {
                continue;
            }
            if (now - input.timestamp > 0.1) {
                g_inputsDropped++;
                continue;
            }
            if (input.down) {
                if (g_lastClickDown && (input.timestamp - g_lastClickTime) < 0.008) {
                    continue;
                }
                g_lastClickTime = input.timestamp;
                g_lastClickDown = true;
            } else {
                g_lastClickDown = false;
            }
            g_totalInputsProcessed++;
            processedThisFrame++;
        }
        while (g_inputQueue.size() > 20000) {
            g_inputQueue.pop_front();
            g_inputsDropped++;
        }
    }
}

$on_mod(Loaded) {
    log::info("CFS Perfect FPS Loading");
    CFS::g_realHz = CFS::detectRealHz();
    log::info("CFS Detected Hz: {}", CFS::g_realHz);
    auto saveDir = Mod::get()->getSaveDir();
    std::error_code ec;
    std::filesystem::create_directories(saveDir, ec);
    CFS::g_lastFPSTime = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    CFS::g_initialized = true;
    log::info("CFS Loaded Correctly");
}

class $modify(CFSSuperKB, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool down, bool repeat) {
        if (CFS::g_initialized && Mod::get()->getSettingValue<bool>("super-inputs")) {
            if (!repeat) {
                std::lock_guard<std::mutex> lock(CFS::g_inputMutex);
                double now = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
                CFS::g_inputQueue.push_back({now, down, static_cast<int>(key), false, CFS::g_nextInputId++});
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, repeat);
    }
}

class $modify(CFSPerfectParticle, cocos2d::CCParticleSystem) {
    bool initWithTotalParticles(unsigned int num, bool p2) {
        if (Mod::get()->getSettingValue<bool>("perfect-fps")) {
            if (static_cast<int>(num) > CFS::g_particleBudget) {
                num = static_cast<unsigned int>(CFS::g_particleBudget);
            }
        }
        return CCParticleSystem::initWithTotalParticles(num, p2);
    }
}

class $modify(CFSPlayerFix, PlayerObject) {
    void pushButton(int button) {
        if (!Mod::get()->getSettingValue<bool>("micro-bugs-fix")) {
            return PlayerObject::pushButton(button);
        }
        double now = std::chrono::duration<double>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
        if (CFS::g_lastClickDown && (now - CFS::g_lastClickTime) < 0.008) {
            return;
        }
        CFS::g_lastClickTime = now;
        CFS::g_lastClickDown = true;
        return PlayerObject::pushButton(button);
    }

    void releaseButton(int button) {
        CFS::g_lastClickDown = false;
        return PlayerObject::releaseButton(button);
    }

    void update(float dt) {
        PlayerObject::update(dt);
        if (!CFS::g_initialized) return;
        if (!Mod::get()->getSettingValue<bool>("micro-bugs-fix")) return;
        CFS::g_prevPlayerX = CFS::g_lastPlayerX;
        CFS::g_prevPlayerY = CFS::g_lastPlayerY;
        CFS::g_lastPlayerX = this->getPositionX();
        CFS::g_lastPlayerY = this->getPositionY();
        float dx = CFS::g_lastPlayerX - CFS::g_prevPlayerX;
        float dy = CFS::g_lastPlayerY - CFS::g_prevPlayerY;
        if (dx == 0.0f && dy == 0.0f) {
            CFS::g_stuckFrames++;
            if (CFS::g_stuckFrames > 3) {
                this->setPosition(CFS::g_lastPlayerX, CFS::g_lastPlayerY + 0.5f);
                CFS::g_stuckFrames = 0;
            }
        } else {
            CFS::g_stuckFrames = 0;
        }
    }
}

class $modify(CFSExtrapolateLayer, PlayLayer) {
    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);
        if (!CFS::g_initialized) return;
        CFS::updateFPSCounter();
        if (Mod::get()->getSettingValue<bool>("super-inputs")) {
            CFS::processInputQueue();
        }
        if (Mod::get()->getSettingValue<bool>("extrapolate")) {
            float dx = CFS::g_lastPlayerX - CFS::g_prevPlayerX;
            float dy = CFS::g_lastPlayerY - CFS::g_prevPlayerY;
            CFS::g_extrapolatedX = CFS::g_lastPlayerX + dx * 0.5f;
            CFS::g_extrapolatedY = CFS::g_lastPlayerY + dy * 0.5f;
        }
    }
}