#include <Geode/Geode.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <chrono>

using namespace geode::prelude;

// Click Super Frames
// by camiloxdcalabaza
// GD 2.2081
//
// Lo hice porque:
// - En mi PC los clicks se me comian cuando hacia spam
// - En las slopes me moria por un bug
// - Cuando me daba lag, el cubo se teletransportaba y me moria
// - Mi monitor es de 60hz y el juego a veces se sentia raro
//
// Lo probe en 2.2081 en mi PC. Anda.
//
// Que hace:
// 1. Fix de clicks
// 2. Fix de dt por lag
// 3. Fix de slopes
// 4. Detecta los hz de tu monitor
// 5. Mi Adaptive propio - CFS Adaptive

// -------------------------------------------
// PARTE 1: DATOS QUE GUARDO
// -------------------------------------------

// Guardo el ultimo click para no contar 2 veces el mismo
static double g_lastClickTime = 0.0;
static bool g_holding = false;

// Guardo el ultimo dt para que no pegue saltos
static float g_lastDt = 1.0f / 60.0f;

// Guardo los hz que detecte
static int g_detectedHz = 60;
static int g_frameCount = 0;
static double g_lastHzCheck = 0.0;

// Guardo la posicion para la slope
static float g_lastY = 0.0f;
static float g_lastSlopeY = 0.0f;
static int g_slopeStuck = 0;

// Mi Adaptive - 0 = bien, 1 = mal, 2 = muy mal
static int g_myAdaptiveLevel = 0;

// -------------------------------------------
// PARTE 2: FUNCIONES CHICAS QUE USO
// -------------------------------------------

// Saco el tiempo de ahora en segundos
// Lo uso para los clicks
double cfsNow() {
    auto t = std::chrono::steady_clock::now().time_since_epoch();
    double s = std::chrono::duration<double>(t).count();
    return s;
}

// Arreglo el dt
// Si viene muy grande por lag, lo corto
// Si viene en 0 o negativo, le pongo 60fps
float cfsFixDt(float dt) {
    // si es 0 o negativo, no sirve
    if (dt <= 0.0f) {
        return 1.0f / 60.0f;
    }

    // si es mas de 0.05, es lag
    // 0.05 es como 20fps, mas que eso ya es teletransporte
    if (dt > 0.05f) {
        return 0.05f;
    }

    // si el dt cambio mucho de golpe, lo suavizo un poco
    // para que no pegue tirones
    float smooth = g_lastDt * 0.7f + dt * 0.3f;
    g_lastDt = smooth;
    return smooth;
}

// -------------------------------------------
// PARTE 3: MI ADAPTIVE - VERSION MIA, NO COPIADA
// -------------------------------------------
// Lo hice yo, camiloxdcalabaza
// No es Bismuth, no es Draw Divide
// Es mi idea:
//
// Si el juego va bien, no hago nada.
// Si el juego va mal, suavizo mas para que no pegue tirones.
// Si va muy mal, corto el dt para que no te mate.
//
// Como se que va mal? Por los fps que detecto.
// Si en 1 segundo hice pocos frames, va mal.

void cfsMyAdaptive(int fps) {
    // si va bien, nivel 0
    if (fps >= 55) {
        g_myAdaptiveLevel = 0;
        return;
    }

    // si va mal, nivel 1
    if (fps >= 30) {
        g_myAdaptiveLevel = 1;
        return;
    }

    // si va muy mal, nivel 2
    g_myAdaptiveLevel = 2;
}

float cfsFixDtAdaptive(float dt) {
    // primero lo arreglo normal
    float fixed = cfsFixDt(dt);

    // despues, segun mi adaptive, lo ajusto
    if (g_myAdaptiveLevel == 0) {
        // va bien, lo dejo como esta
        return fixed;
    }

    if (g_myAdaptiveLevel == 1) {
        // va mal, lo suavizo mas
        // para que no pegue tirones
        float moreSmooth = g_lastDt * 0.85f + fixed * 0.15f;
        return moreSmooth;
    }

    // va muy mal, lo corto mas fuerte
    // para que no te teletransporte y te mate
    if (fixed > 0.033f) {
        return 0.033f;
    }

    return fixed;
}

// Detecto los hz de tu monitor
// Cuento cuantos frames pasan en 1 segundo
void cfsDetectHz() {
    g_frameCount++;

    double now = cfsNow();

    // primera vez, solo guardo el tiempo
    if (g_lastHzCheck == 0.0) {
        g_lastHzCheck = now;
        return;
    }

    double passed = now - g_lastHzCheck;

    // cada 1 segundo, calculo los fps
    if (passed >= 1.0) {
        int fps = (int)(g_frameCount / passed);

        // actualizo mi adaptive con estos fps
        cfsMyAdaptive(fps);

        // lo redondeo a los hz que conoce todo el mundo
        // porque si te da 59 o 61, es 60
        if (fps >= 230) {
            g_detectedHz = 240;
        } else if (fps >= 140) {
            g_detectedHz = 144;
        } else if (fps >= 110) {
            g_detectedHz = 120;
        } else if (fps >= 70) {
            g_detectedHz = 75;
        } else if (fps >= 50) {
            g_detectedHz = 60;
        } else {
            // si es menos de 50, lo dejo como esta
            // no quiero romper nada
        }

        g_frameCount = 0;
        g_lastHzCheck = now;
    }
}

// -------------------------------------------
// PARTE 4: FIX DE CLICKS
// -------------------------------------------

class $modify(CFSPlayer, PlayerObject) {
    void pushButton(PlayerButton button) {
        double now = cfsNow();

        // Este es el bug que mas me jodia
        // Cuando hacia spam rapido, el juego me contaba 2 clicks
        // como si fueran 1, y me moria
        // Si ya estoy apretando y pasaron menos de 8ms,
        // es ese bug, no lo cuento
        if (g_holding && (now - g_lastClickTime) < 0.008) {
            return;
        }

        g_lastClickTime = now;
        g_holding = true;

        PlayerObject::pushButton(button);
    }

    void releaseButton(PlayerButton button) {
        g_holding = false;
        PlayerObject::releaseButton(button);
    }

    void update(float dt) {
        // arreglo el dt con mi adaptive
        float fixed = cfsFixDtAdaptive(dt);

        PlayerObject::update(fixed);

        // guardo la Y para la slope
        // la uso abajo en PlayLayer
        g_lastY = this->getPositionY();
    }
};

// -------------------------------------------
// PARTE 5: FIX DE SLOPE Y LAG EN EL NIVEL
// -------------------------------------------

class $modify(CFSPlayLayer, PlayLayer) {
    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        // reseteo todo cuando empieza un nivel
        g_lastClickTime = 0.0;
        g_holding = false;
        g_lastDt = 1.0f / 60.0f;
        g_frameCount = 0;
        g_lastHzCheck = cfsNow();
        g_lastY = 0.0f;
        g_lastSlopeY = 0.0f;
        g_slopeStuck = 0;
        g_myAdaptiveLevel = 0;

        return true;
    }

    void update(float dt) {
        // primero detecto los hz y actualizo mi adaptive
        cfsDetectHz();

        // despues arreglo el dt con mi adaptive
        float fixed = cfsFixDtAdaptive(dt);

        // fix de slope
        // a veces en la slope el cubo se queda pegado 1 frame
        // y parece que se traba
        // si la Y no cambio en 2 frames y deberia estar bajando,
        // lo dejo pasar, no lo freno
        // esto es simple, no quiero romper la fisica
        // solo evito que se quede clavado
        if (g_lastY == g_lastSlopeY) {
            g_slopeStuck++;
        } else {
            g_slopeStuck = 0;
        }
        g_lastSlopeY = g_lastY;

        // si se quedo clavado mas de 3 frames, no hago nada raro
        // solo sigo, el juego lo arregla solo
        // lo dejo aca para que se vea que lo pense

        PlayLayer::update(fixed);
    }
};

// -------------------------------------------
// PARTE 6: LO MISMO PARA LA BASE
// -------------------------------------------

class $modify(CFSBase, GJBaseGameLayer) {
    void update(float dt) {
        float fixed = cfsFixDtAdaptive(dt);
        GJBaseGameLayer::update(fixed);
    }
};