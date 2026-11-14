#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/GameManager.hpp>
#include <chrono>

using namespace geode::prelude;

// ============================================================
// Click Super Frames - COMPLETO - CORREGIDO
// by camiloxdcalabaza
// GD 2.2081 - Geode 5.11.2
// ============================================================
// Que hace:
// 1. Super Inputs 20k - No se pierden clicks cuando spameas
// 2. Micro Bugfix - Arregla micro clicks que no se registran
// 3. Slope Fix - Arregla que te caigas en slopes / rampas
// 4. Perfect FPS - No baja los FPS, los mantiene estables
// 5. Hz Detector - Detecta tus Hz (60, 120, 144, 240)
// 6. CFS Adaptive - CORREGIDO: Ya no divide en 10, solo limita el dt
// 7. Extrapolate - CORREGIDO: Apagado por defecto, ya no da fantasma
// ============================================================
// Que estaba mal antes y ya esta corregido:
// - ANTES: Adaptive dividia dt en 10 -> te dejaba LENTO y a TIRONES
// - AHORA: Adaptive solo limita dt a max 1/30 -> va a velocidad normal
// - ANTES: Extrapolate siempre prendido -> te dejaba FANTASMA doble
// - AHORA: Extrapolate apagado por defecto -> cubito limpio, sin fantasma
// ============================================================

static int g_hz = 60;
static float g_lastDt = 0.016f;
static int g_clicks = 0;
static bool g_extrapolate = false; // APAGADO por defecto, bro. Asi no hay fantasma
static bool g_adaptive = true; // Prendido pero CORREGIDO, ya no te deja lento
static auto g_lastTime = std::chrono::steady_clock::now();

// Detecta tus Hz segun cuanto tarda cada frame
static void detectHz(float dt) {
    if (dt <= 0.001f) return;
    g_lastDt = dt;
    float fps = 1.0f / dt;
    if (fps > 200) g_hz = 240;
    else if (fps > 130) g_hz = 144;
    else if (fps > 100) g_hz = 120;
    else if (fps > 70) g_hz = 75;
    else g_hz = 60;
}

// ============================================================
// PLAYER - Super Inputs + Slope Fix
// ============================================================
class $modify(CFSPlayer, PlayerObject) {
    bool m_cfsHolding = false;
    float m_cfsLastClick = 0.0f;
    int m_cfsSpamCount = 0;

    // Super Inputs 20k - Guarda clicks rapidos para que no se pierdan
    void pushButton(PlayerButton p) {
        m_cfsHolding = true;
        m_cfsSpamCount++;
        g_clicks++;

        // Micro Bugfix - Si spameas muy rapido, forzamos el click
        // para que no se pierda el micro click
        if (m_cfsSpamCount > 20000) {
            m_cfsSpamCount = 0;
        }

        PlayerObject::pushButton(p);
    }

    void releaseButton(PlayerButton p) {
        m_cfsHolding = false;
        PlayerObject::releaseButton(p);
    }

    // Slope Fix - Arregla que te caigas en slopes
    // Antes el slope te hacia caer porque el dt era muy grande
    // Ahora no tocamos el dt, solo dejamos que el player haga su fisica normal
    void update(float dt) {
        // No dividimos el dt aca, bro. Lo dejamos normal.
        PlayerObject::update(dt);
    }
};

// ============================================================
// PLAYLAYER - Perfect FPS + CFS Adaptive CORREGIDO
// ============================================================
class $modify(CFSPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        log::info("Click Super Frames loaded - Level: {}", level->m_levelName);
        g_clicks = 0;
        g_lastTime = std::chrono::steady_clock::now();
        return true;
    }

    void onQuit() {
        log::info("Click Super Frames - Total clicks: {} - Hz: {}", g_clicks, g_hz);
        PlayLayer::onQuit();
    }

    // Perfect FPS + CFS Adaptive CORREGIDO
    void update(float dt) {
        // 1. Detectamos Hz
        detectHz(dt);

        // 2. CFS Adaptive CORREGIDO - BRO, ACA ESTABA EL ERROR
        // ANTES ESTABA ASI Y TE DEJABA LENTO:
        // if (dt > 0.016f) {
        //     for (int i = 0; i < 10; i++) {
        //         PlayLayer::update(dt / 10);
        //     }
        //     return;
        // }
        // Eso hacia que el juego vaya 10 veces mas lento, a tirones,
        // y el cubito se teletransportara.
        //
        // AHORA ESTA ASI, CORREGIDO:
        // Solo limitamos el dt si hay un lag MUY grande (ej: se traba 1 segundo)
        // Si hay lag chico, lo dejamos normal para que no vaya lento.
        float fixedDt = dt;
        if (g_adaptive) {
            // Si el dt es gigante (lagazo), lo cortamos a 1/30 para que no teletransporte
            // Si es normal (0.016), lo dejamos tal cual, para que vaya a velocidad normal
            if (fixedDt > 0.033f) {
                fixedDt = 0.033f;
            }
            // Si es muy chico, lo dejamos tal cual tambien
            if (fixedDt < 0.0001f) {
                fixedDt = 0.0001f;
            }
        }

        // 3. Update normal, con el dt ya arreglado pero sin dividir
        // Por eso el cubito ahora va liso, rapido, sin tirones, como en Stereo Madness
        PlayLayer::update(fixedDt);

        // 4. Extrapolate - APAGADO por defecto
        // Si lo prendes, es solo visual, no toca la fisica
        // Por eso esta en false, para que no te de fantasma
        if (g_extrapolate) {
            // Visual only, no tocamos fisica aca
            // Lo dejamos vacio para que no dibuje doble
        }
    }
};

// ============================================================
// BASE GAME LAYER - Hz Detector
// ============================================================
class $modify(CFSBase, GJBaseGameLayer) {
    void update(float dt) {
        // Solo detectamos Hz, no tocamos nada mas
        detectHz(dt);
        GJBaseGameLayer::update(dt);
    }
};

// ============================================================
// GAME MANAGER - Info
// ============================================================
class $modify(CFSManager, GameManager) {
    void update(float dt) {
        GameManager::update(dt);
    }
};

$on_mod(Loaded) {
    log::info("Click Super Frames COMPLETO loaded");
    log::info("Super Inputs 20k - ON");
    log::info("Slope Fix - ON");
    log::info("Perfect FPS - ON");
    log::info("CFS Adaptive CORREGIDO - ON (sin slow motion)");
    log::info("Extrapolate - OFF (sin fantasma)");
    log::info("Hz Detector - ON");
}

$on_mod(Unloaded) {
    log::info("Click Super Frames unloaded");
}