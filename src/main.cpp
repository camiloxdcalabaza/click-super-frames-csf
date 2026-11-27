#include <Geode/Geode.hpp>
#include <queue>
#include <chrono>

using namespace geode::prelude;

// Click Super Frames
// idea simple: el juego corre a 60/120/240 fps pero tu mouse
// clickea cuando quiere. Si clickeas entre 2 frames, vanilla
// lo pierde y se siente como "aprete y no salto".
// aca guardo los clicks con su hora y los meto en el update.

struct Click {
    bool abajo;
    double cuando;
    int boton;
};

static std::queue<Click> cola;

// este es el que ya le metimos al juego en update
static bool sosteniendo = false;
// este es el que esta en la cola esperando a entrar
// si no lo separo, el doble click entra 2 veces antes del update
static bool apretadoPendiente = false;

static double ultimoReal = 0;
static bool ultimoEstado = false;
static bool primerClick = true;

// cache del modo, para no pedir el setting en cada frame
static std::string modoCache = "cbf";
static double ultimoCheckModo = 0;

static double ultimoCos = 0;
static double inicioSeg = 0;
static int clicksSeg = 0;

// para la pausa: si pausas apretando y soltas en el menu,
// el juego queda creyendo que seguis apretando
static bool pausoApretado = false;
static double horaResume = 0;
static bool esperandoSoltarPostPausa = false;

static double ahora() {
    auto t = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration<double>(t).count();
}

static void limpiar() {
    while (!cola.empty()) cola.pop();
    sosteniendo = false;
    apretadoPendiente = false;
    clicksSeg = 0;
    pausoApretado = false;
    esperandoSoltarPostPausa = false;
}

static std::string modo() {
    double t = ahora();
    // reviso 1 vez por segundo, no 60 llamadas por frame
    if (t - ultimoCheckModo > 1.0 || modoCache.empty()) {
        ultimoCheckModo = t;
        modoCache = Mod::get()->getSettingValue<std::string>("input-mode");
    }
    return modoCache;
}

$on_mod(Loaded) {
    log::info("CFS cargado");
    modoCache = Mod::get()->getSettingValue<std::string>("input-mode");
    ultimoCheckModo = ahora();
    limpiar();
}

class $modify(PlayLayer) {

    void update(float dt) {
        std::string m = modo();

        // modo off con cola llena
        // si pasas a off con un click en cola, esa cola nunca se vacia
        // y cuando volves a cbf te entra un click viejo de golpe
        if (m == "off") {
            if (!cola.empty()) {
                limpiar();
            }
            PlayLayer::update(dt);
            return;
        }

        {
            double tAhora = ahora();

            if (tAhora - inicioSeg >= 1.0) {
                inicioSeg = tAhora;
                clicksSeg = 0;
            }

            // si volvi de pausa apretando y pasaron 0.3s sin que llegue
            // un soltar real, es porque lo solto en el menu. Lo suelto yo.
            // si sigue apretando de verdad, el juego manda otro push y no entra aca
            if (esperandoSoltarPostPausa && tAhora - horaResume > 0.3) {
                esperandoSoltarPostPausa = false;
                if (sosteniendo && !apretadoPendiente) {
                    cola.push({false, tAhora, 0});
                }
            }

            while (!cola.empty()) {
                Click c = cola.front();
                cola.pop();

                // COS: uso c.cuando, no tAhora
                if (m == "cos" && c.abajo) {
                    if (c.cuando - ultimoCos < 1.0 / 240.0) {
                        // si lo descarto, libero el pendiente
                        // si no, el proximo click no entra
                        apretadoPendiente = sosteniendo;
                        continue;
                    }
                    if (clicksSeg >= 240) {
                        apretadoPendiente = sosteniendo;
                        continue;
                    }
                    ultimoCos = c.cuando;
                    clicksSeg++;
                }

                // OJO: aca va PlayLayer::, no this->
                // this->pushButton llama a MI pushButton y me vuelve
                // a meter el click en la cola, loop infinito
                if (c.abajo) {
                    if (sosteniendo) {
                        apretadoPendiente = sosteniendo;
                        continue;
                    }
                    PlayLayer::pushButton(c.boton, true);
                    sosteniendo = true;
                } else {
                    if (!sosteniendo) {
                        apretadoPendiente = false;
                        continue;
                    }
                    PlayLayer::releaseButton(c.boton, true);
                    sosteniendo = false;
                }
                // despues de aplicar, pendiente = lo que quedo
                // este es el UNICO lugar que sincroniza pendiente con sosteniendo
                apretadoPendiente = sosteniendo;
            }
        }

        PlayLayer::update(dt);
    }

    void pushButton(int id, bool down) {
        std::string m = modo();

        if (m == "off" || id != 0) {
            PlayLayer::pushButton(id, down);
            return;
        }

        double t = ahora();

        // aca NO uso 'down' para saber el estado.
        // en pushButton, el estado siempre es abajo = true.
        // el 'down' que manda el juego no es apretado/soltado.
        if (!primerClick && ultimoEstado == true && t - ultimoReal < 0.003) {
            return; // apretar-apretar en <3ms = mouse berreta
        }

        // si ya hay un apretar en cola, no meto otro
        if (apretadoPendiente) return;

        primerClick = false;
        ultimoReal = t;
        ultimoEstado = true;
        apretadoPendiente = true;

        // si volvi de pausa y llega un push real, era que seguia apretando
        // cancelo el soltar automatico
        esperandoSoltarPostPausa = false;

        cola.push({true, t, id});
    }

    void releaseButton(int id, bool down) {
        std::string m = modo();

        if (m == "off" || id != 0) {
            PlayLayer::releaseButton(id, down);
            return;
        }

        double t = ahora();

        // mismo arreglo: en release, el estado siempre es arriba = false
        if (!primerClick && ultimoEstado == false && t - ultimoReal < 0.003) {
            return; // soltar-soltar en <3ms = fantasma
        }

        primerClick = false;
        ultimoReal = t;
        ultimoEstado = false;

        // FIX releaseButton: antes aca ponia apretadoPendiente = false
        // apenas llegaba el soltar, aunque todavia habia un apretar
        // en cola sin procesar. Eso dejaba entrar un apretar duplicado
        // si el mouse mandaba otro push en esos milisegundos.
        // Ahora NO lo toco. Lo sincroniza solo el update al final del while.
        // Si no hay apretar en cola, ya vale false de antes, no cambia nada.

        esperandoSoltarPostPausa = false;

        // si no hay nada apretado ni en cola, no guardo un soltar al pedo
        if (!sosteniendo && cola.empty()) return;

        cola.push({false, t, id});
    }

    void resetLevel() {
        // si moris con el click apretado, revivis saltando solo
        // lo vi mil veces en niveles de wave
        limpiar();
        PlayLayer::resetLevel();
    }

    void onQuit() {
        limpiar();
        PlayLayer::onQuit();
    }

    void pauseGame(bool p0) {
        pausoApretado = sosteniendo || apretadoPendiente;
        PlayLayer::pauseGame(p0);
    }

    void resume() {
        if (pausoApretado) {
            // no suelto aca de una, porque capaz sigue apretando
            // espero 0.3s en update y si no llego soltar real, lo suelto yo
            esperandoSoltarPostPausa = true;
            horaResume = ahora();
            pausoApretado = false;
        }
        PlayLayer::resume();
    }

    void onPracticeMode(bool p0) {
        limpiar();
        PlayLayer::onPracticeMode(p0);
    }
};

class $modify(GJBaseGameLayer) {
    void loadCheckpoint() {
        // si no limpio aca, el click de antes de morir te hace
        // saltar apenas revivis en el checkpoint
        limpiar();
        GJBaseGameLayer::loadCheckpoint();
    }

    void deleteCheckpoint() {
        limpiar();
        GJBaseGameLayer::deleteCheckpoint();
    }
};