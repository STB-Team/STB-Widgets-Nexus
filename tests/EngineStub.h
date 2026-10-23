#pragma once
// Test double for the small engine surface used by HudRuntime.cpp. The actual
// production translation unit is compiled into these tests; this is not Skyrim.
#include <algorithm>
#include <cstdint>
#include <functional>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

namespace logger { template<class... T> void info(const char*, T...) {} }
namespace RE
{
#define STB_TEST_MENU(type, name) struct type { static constexpr std::string_view MENU_NAME = name; }
STB_TEST_MENU(LoadingMenu, "Loading Menu");
STB_TEST_MENU(FaderMenu, "Fader Menu");
STB_TEST_MENU(MainMenu, "Main Menu");
STB_TEST_MENU(InventoryMenu, "InventoryMenu");
STB_TEST_MENU(CraftingMenu, "Crafting Menu");
STB_TEST_MENU(BarterMenu, "BarterMenu");
STB_TEST_MENU(TweenMenu, "TweenMenu");
STB_TEST_MENU(GiftMenu, "GiftMenu");
STB_TEST_MENU(ContainerMenu, "ContainerMenu");
STB_TEST_MENU(MagicMenu, "MagicMenu");
STB_TEST_MENU(JournalMenu, "Journal Menu");
STB_TEST_MENU(LockpickingMenu, "Lockpicking Menu");
STB_TEST_MENU(RaceSexMenu, "RaceSex Menu");
STB_TEST_MENU(MapMenu, "MapMenu");
STB_TEST_MENU(DialogueMenu, "Dialogue Menu");
STB_TEST_MENU(CursorMenu, "Cursor Menu");
STB_TEST_MENU(MessageBoxMenu, "MessageBoxMenu");
#undef STB_TEST_MENU

struct UI
{
    static UI* GetSingleton();
    bool IsMenuOpen(std::string_view name) const { return open.count(std::string(name)) != 0; }
    bool GameIsPaused() const { return paused; }
    std::set<std::string> open;
    bool paused = false;
};
struct GFxValue
{
    GFxValue() = default;
    explicit GFxValue(float value) : number(value), numeric(true) {}
    bool IsNumber() const { return numeric; }
    double GetNumber() const { return number; }
    double number = 0;
    bool numeric = false;
};
struct GFxMovieView
{
    void CheckThread() const
    {
        if (owner != std::this_thread::get_id()) {
            throw std::runtime_error("GFx touched on producer thread");
        }
    }
    void SetVisible(bool value) { CheckThread(); visible = value; calls.push_back("visible"); }
    bool GetVariable(GFxValue* value, const char*)
    {
        CheckThread();
        *value = GFxValue(alpha);
        value->numeric = alphaIsNumber;
        return hasAlpha;
    }
    void Invoke(const char*, void*, const GFxValue* value, int)
    {
        CheckThread();
        alpha = static_cast<float>(value->GetNumber());
        calls.push_back("alpha");
    }
    std::thread::id owner = std::this_thread::get_id();
    std::vector<std::string> calls;
    bool visible = false;
    bool hasAlpha = true;
    bool alphaIsNumber = true;
    float alpha = 100;
};
struct IMenu
{
    void AdvanceMovie(float, std::uint32_t)
    {
        uiMovie->CheckThread();
        uiMovie->calls.push_back("advance");
        if (duringAdvance) { duringAdvance(); }
    }
    std::shared_ptr<GFxMovieView> uiMovie = std::make_shared<GFxMovieView>();
    std::function<void()> duringAdvance;
};
}
