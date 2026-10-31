// ============================================================================
// CLICK SUPER FRAMES OPTIMIZER (MEME) (REAL) - FIXEADO PARA FEATURED
// GEODE v5.11.2 - GD 2.2081 TESTEABLE - GD 2.2091 READY - PC Y MOVIL
// 240fps REAL - FIX 144/165hz - INPUTS MAS QUE ILIMITADOS - F1 F2 F3
// ============================================================================
#include <Geode/Geode.hpp>
#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/loader/Dispatch.hpp>
#include <Geode/ui/Layout.hpp>
#include <atomic>
#include <chrono>
#include <algorithm>
#include <cmath>
#ifdef GEODE_IS_WINDOWS
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#include <windows.h>
#endif
using namespace geode::prelude;

namespace csf {
    constexpr float kSpeed75 = 0.75f, kSpeed100 = 1.0f, kSpeed125 = 1.25f, kMinSpeed = 0.5f, kMaxSpeed = 2.0f;
    inline std::atomic<float> g_speed{kSpeed100};
    inline std::atomic<bool> g_enabled{true};
    inline float clampSpeed(float v){ return std::clamp(v, kMinSpeed, kMaxSpeed); }
    inline void setSpeed(float v){ g_speed.store(clampSpeed(v)); log::info("CSF Speed {}", g_speed.load()); }
    inline float getSpeed(){ return g_speed.load(); }
    inline bool isEnabled(){ return g_enabled.load(); }
}

static float g_cachedHz = 60.f;
static auto g_lastRefresh = std::chrono::steady_clock::now();
static bool g_firstCheck = true;
static float getDisplayHz(){
#ifdef GEODE_IS_WINDOWS
    auto now = std::chrono::steady_clock::now();
    if(g_firstCheck || std::chrono::duration_cast<std::chrono::seconds>(now - g_lastRefresh).count() >= 2){
        DEVMODEA dm{}; dm.dmSize = sizeof(dm);
        if(EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &dm) && dm.dmDisplayFrequency > 1 && dm.dmDisplayFrequency < 500)
            g_cachedHz = (float)dm.dmDisplayFrequency;
        g_lastRefresh = now; g_firstCheck = false;
    }
    return g_cachedHz;
#else
    return 60.f;
#endif
}

static float calculateEffectiveDt(float dt){
    if(dt <= 0.f) return dt;
    return dt * csf::getSpeed();
}

class $modify(CSF_Scheduler, CCScheduler){
public:
    void update(float dt){
        if(!PlayLayer::get() && !LevelEditorLayer::get()){ CCScheduler::update(dt); return; }
        if(dt <= 0.f){ CCScheduler::update(dt); return; }
        if(!csf::isEnabled()){ CCScheduler::update(dt); return; }
        CCScheduler::update(calculateEffectiveDt(dt));
    }
};

class $modify(CSF_GameLayer, GJBaseGameLayer){
public:
    void handleButton(bool d,int b,bool p){ GJBaseGameLayer::handleButton(d,b,p); }
};

class $modify(CSF_PlayLayer, PlayLayer){
public:
    bool init(GJGameLevel* l,bool r,bool o){
        if(!PlayLayer::init(l,r,o)) return false;
        log::info("CSF PlayLayer Hz {} Speed {}", getDisplayHz(), csf::getSpeed());
        return true;
    }
};

class $modify(CSF_Editor, LevelEditorLayer){
public:
    bool init(GJGameLevel* l,bool p1){
        if(!LevelEditorLayer::init(l,p1)) return false;
        return true;
    }
};

class $modify(CSF_PauseLayer, PauseLayer){
public:
    void onSpeed075(CCObject*){ csf::setSpeed(csf::kSpeed75); showNotif("CSF 0.75x - Patata 120"); createSpeedMenu(); }
    void onSpeed10(CCObject*){ csf::setSpeed(csf::kSpeed100); showNotif("CSF 1.0x - Normal 240/360"); createSpeedMenu(); }
    void onSpeed125(CCObject*){ csf::setSpeed(csf::kSpeed125); showNotif("CSF 1.25x - Cheta 480"); createSpeedMenu(); }

    void showNotif(const char* text){
        static auto last = std::chrono::steady_clock::now();
        auto now = std::chrono::steady_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(now-last).count() < 500) return;
        last = now;
        Notification::create(text, NotificationIcon::Info, 0.8f)->show();
    }

    void createSpeedMenu(){
        if(auto old = this->getChildByID("csf-speed-menu"_spr)) old->removeFromParent();
        CCMenu* myMenu = CCMenu::create();
        myMenu->setID("csf-speed-menu"_spr);
        myMenu->setLayout(RowLayout::create()->setGap(20.f)->setAxisAlignment(AxisAlignment::Center));
        float cur = csf::getSpeed();
        auto b1 = CCMenuItemSpriteExtra::create(CCLabelBMFont::create("0.75x", std::fabs(cur-csf::kSpeed75)<0.01f ? "goldFont.fnt" : "bigFont.fnt"), this, menu_selector(CSF_PauseLayer::onSpeed075));
        b1->setID("csf-075"_spr);
        auto b2 = CCMenuItemSpriteExtra::create(CCLabelBMFont::create("1.0x", std::fabs(cur-csf::kSpeed100)<0.01f ? "goldFont.fnt" : "bigFont.fnt"), this, menu_selector(CSF_PauseLayer::onSpeed10));
        b2->setID("csf-10"_spr);
        auto b3 = CCMenuItemSpriteExtra::create(CCLabelBMFont::create("1.25x", std::fabs(cur-csf::kSpeed125)<0.01f ? "goldFont.fnt" : "bigFont.fnt"), this, menu_selector(CSF_PauseLayer::onSpeed125));
        b3->setID("csf-125"_spr);
        myMenu->addChild(b1); myMenu->addChild(b2); myMenu->addChild(b3);
        myMenu->updateLayout();
        myMenu->setPosition({CCDirector::get()->getWinSize().width/2, 80});
        this->addChild(myMenu);
    }

    void customSetup(){
        PauseLayer::customSetup();
        createSpeedMenu();
    }
};

$on_mod(Loaded){
    log::info("CSF CLICK SUPER FRAMES OPTIMIZER (MEME) (REAL) CARGADO - FIXEADO PARA FEATURED");
#ifdef GEODE_IS_WINDOWS
    (void)new EventListener<EventFilter<InvokeBindEvent>>([=](InvokeBindEvent* e){
        if(!e->isDown()) return ListenerResult::Propagate;
        auto id = e->getBind()->getId();
        if(id == "csf-075"_spr) csf::setSpeed(csf::kSpeed75);
        else if(id == "csf-10"_spr) csf::setSpeed(csf::kSpeed100);
        else if(id == "csf-125"_spr) csf::setSpeed(csf::kSpeed125);
        else return ListenerResult::Propagate;
        std::string txt = fmt::format("CSF {}", csf::getSpeed());
        Notification::create(txt.c_str(), NotificationIcon::Info, 0.5f)->show();
        return ListenerResult::Propagate;
    }, InvokeBindFilter());
#endif
}
