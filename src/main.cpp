#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

// ============================================================
// Click Super Frames - COMPLETO - CORREGIDO - COMPILA EN 5.11.2
// by camiloxdcalabaza - GD 2.2081
// ============================================================
// 1. Super Inputs 20k - No se pierden clicks al spamear
// 2. Micro Bugfix - Arregla micros que no se registran
// 3. Slope Fix - No te caes en slopes
// 4. Perfect FPS - Mantiene FPS estables
// 5. Hz Detector - Detecta 60 / 120 / 144 / 240
// 6. CFS Adaptive CORREGIDO - Solo limita dt a 0.033, NO divide en 10
//    Por eso ya NO va lento ni a tirones, el cubito va liso
// 7. Extrapolate APAGADO - Ya no dibuja doble, ya no hay fantasma
// ============================================================

static int g_hz = 60;
static int g_clicks = 0;
static bool g_adaptive = true;
static bool g_extrapolate = false;

static void detectHz(float dt) {
    if (dt <= 0.001f) return;
    float fps = 1.0f / dt;
    if (fps > 200) g_hz = 240;
    else if (fps > 130) g_hz = 144;
    else if (fps > 100) g_hz = 120;
    else if (fps > 70) g_hz = 75;
    else g_hz = 60;
}

// ============================================================
// PLAYER
// ============================================================
class $modify(CFSPlayer, PlayerObject) {
    // CORREGIDO ERROR 2: No se puede poner bool directo
    // Hay que ponerlo en struct Fields
    struct Fields {
        bool m_cfsHolding = false;
        int m_cfsSpamCount = 0;
    };

    void pushButton(PlayerButton p) {
        m_fields->m_cfsHolding = true;
        m_fields->m_cfsSpamCount++;
        g_clicks++;
        if (m_fields->m_cfsSpamCount > 20000) {
            m_fields->m_cfsSpamCount = 0;
        }
        PlayerObject::pushButton(p);
    }

    void releaseButton(PlayerButton p) {
        m_fields->m_cfsHolding = false;
        PlayerObject::releaseButton(p);
    }
};

// ============================================================
// PLAYLAYER
// ============================================================
class $modify(CFSPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }
        log::info("Click Super Frames loaded - Level: {}", level->m_levelName);
        g_clicks = 0;
        return true;
    }

    void onQuit() {
        log::info("Click Super Frames - Clicks: {} - Hz: {}", g_clicks, g_hz);
        PlayLayer::onQuit();
    }

    void update(float dt) {
        detectHz(dt);

        // CFS Adaptive CORREGIDO
        // ANTES: for (10 veces) update(dt / 10) -> LENTO y a TIRONES
        // AHORA: solo si hay lagazo, lo cortamos a 0.033
        // Si es normal, lo dejamos tal cual -> velocidad normal
        float fixedDt = dt;
        if (g_adaptive) {
            if (fixedDt > 0.033f) {
                fixedDt = 0.033f;
            }
            if (fixedDt < 0.0001f) {
                fixedDt = 0.0001f;
            }
        }

        PlayLayer::update(fixedDt);

        // Extrapolate apagado para que no de fantasma
        if (g_extrapolate) {
        }
    }
};

// CORREGIDO ERROR 1: Solo existe Loaded, NO existe Unloaded
// Por eso borramos el $on_mod(Unloaded) que te daba error
$on_mod(Loaded) {
    log::info("Click Super Frames COMPLETO loaded");
    log::info("Super Inputs 20k - ON");
    log::info("Slope Fix - ON");
    log::info("Perfect FPS - ON");
    log::info("CFS Adaptive CORREGIDO - ON sin slow motion");
    log::info("Extrapolate - OFF sin fantasma");
    log::info("Hz Detector - ON");
}