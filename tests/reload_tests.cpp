#include "EngineStub.h"
#include "HudRuntime.h"
#include <atomic>
#include <iostream>

namespace {
RE::UI ui;
bool uiExists = true;
bool playerExists = true;
int checks = 0;
void Require(bool result, const char* message)
{
    ++checks;
    if (!result) { throw std::runtime_error(message); }
}
}
RE::UI* RE::UI::GetSingleton() { return uiExists ? &ui : nullptr; }
bool STB::ApplyWidgetConfiguration(Widget, RE::GFxMovieView& movie)
{
    movie.CheckThread();
    if (!playerExists) { return false; }
    Require(!movie.calls.empty() && movie.calls.back() == "advance", "configure before own advance");
    movie.calls.push_back("configure");
    return true;
}

int main()
try {
    using namespace STB;
    auto& hud = Hud();
    Require(!hud.Frame().ready, "must not update before a game is ready");
    RE::IMenu menu;
    WidgetFrameState state;
    Require(!AdvanceWidget(menu, Widget::Level, state, .016f, 1), "cold main menu");
    Require(menu.uiMovie->calls == std::vector<std::string>{"visible"}, "blocked movie advanced");
    hud.NewGame();
    menu.uiMovie->calls.clear();
    Require(AdvanceWidget(menu, Widget::Level, state, .016f, 2), "new game frame");
    Require(menu.uiMovie->calls == std::vector<std::string>({"advance","configure","visible"}), "frame ordering");
    const auto configured = state.configuration;
    menu.uiMovie->calls.clear();
    AdvanceWidget(menu, Widget::Level, state, .016f, 3);
    Require(menu.uiMovie->calls == std::vector<std::string>({"advance","visible"}), "unnecessary preset rebuild");

    // Both legal SKSE post-load / loading-menu-close orderings.
    for (bool postFirst : {false, true}) {
        hud.Alpha(Widget::Level, { 11.0f, std::nullopt });
        const auto oldGeneration = hud.Frame().generation;
        hud.BeginSaveLoad();
        hud.LoadingMenu(true);
        Require(!hud.Frame().ready, "load gate");
        Require(!hud.TakeAlpha(Widget::Level, oldGeneration), "old alpha survived reload");
        hud.Alpha(Widget::Level, { 12.0f, std::nullopt });
        if (postFirst) { hud.FinishSaveLoad(true); } else { hud.LoadingMenu(false); }
        Require(!hud.Frame().ready, "one gate opened early");
        if (postFirst) { hud.LoadingMenu(false); } else { hud.FinishSaveLoad(true); }
        Require(hud.Frame().ready, "both gates should be open");
        Require(!hud.TakeAlpha(Widget::Level, hud.Frame().generation), "loading-time event survived");
        Require(AdvanceWidget(menu, Widget::Level, state, .016f, 4), "reload frame");
        Require(state.configuration != configured, "reload did not reconfigure");
    }

    hud.BeginSaveLoad(); hud.LoadingMenu(false); hud.FinishSaveLoad(false);
    Require(!hud.Frame().ready, "failed load enabled widget updates");
    hud.BeginSaveLoad(); hud.FinishSaveLoad(true);
    Require(hud.Frame().ready, "recovery after failed load");

    // Recreated or delayed widgets configure independently, never gated on equip.
    for (std::size_t i = 0; i < WidgetCount; ++i) {
        RE::IMenu late;
        WidgetFrameState lateState;
        late.uiMovie.reset();
        Require(!AdvanceWidget(late, static_cast<Widget>(i), lateState, .016f, 5), "null movie");
        Require(lateState.configuration == 0, "missing movie acknowledged settings");
        late.uiMovie = std::make_shared<RE::GFxMovieView>();
        Require(AdvanceWidget(late, static_cast<Widget>(i), lateState, .016f, 6), "late widget");
        Require(lateState.configuration == hud.Frame().configuration, "late widget not configured");
    }

    hud.Configure(); playerExists = false;
    Require(!AdvanceWidget(menu, Widget::Level, state, .016f, 7), "missing player");
    Require(state.configuration != hud.Frame().configuration, "missing player lost pending config");
    playerExists = true;
    Require(AdvanceWidget(menu, Widget::Level, state, .016f, 8), "player returned");
    uiExists = false;
    Require(!AdvanceWidget(menu, Widget::Level, state, .016f, 9), "missing UI");
    uiExists = true;

    // Loading reentered during an engine callback: never apply the old batch.
    hud.Configure();
    menu.duringAdvance = [&] { hud.BeginSaveLoad(); };
    Require(!AdvanceWidget(menu, Widget::Level, state, .016f, 10), "mid-frame load");
    Require(state.configuration != hud.Frame().configuration, "mid-frame load configured stale state");
    menu.duringAdvance = {};
    hud.FinishSaveLoad(true);

    for (const auto name : {"Loading Menu","Fader Menu","Main Menu","InventoryMenu","Crafting Menu",
        "BarterMenu","TweenMenu","GiftMenu","ContainerMenu","MagicMenu","Journal Menu",
        "Lockpicking Menu","RaceSex Menu","MapMenu","Dialogue Menu","Cursor Menu","MessageBoxMenu"}) {
        ui.open.insert(name);
        AdvanceWidget(menu, Widget::Level, state, .016f, 11);
        Require(!menu.uiMovie->visible, "blocking menu did not hide widgets");
        ui.open.erase(name);
        AdvanceWidget(menu, Widget::Level, state, .016f, 12);
        Require(menu.uiMovie->visible, "widgets remained hidden after menu closed");
    }
    ui.open.insert("TweenMenu"); ui.open.insert("MapMenu"); ui.open.erase("TweenMenu");
    AdvanceWidget(menu, Widget::Level, state, .016f, 13);
    Require(!menu.uiMovie->visible, "nested menu restored early");
    ui.open.clear();
    hud.Visibility(false);
    AdvanceWidget(menu, Widget::Level, state, .016f, 14);
    Require(!menu.uiMovie->visible, "camera visibility request ignored");
    hud.Visibility(true);
    ui.paused = true;
    hud.Configure();
    Require(!AdvanceWidget(menu, Widget::Level, state, .016f, 15), "paused game data update");
    Require(state.configuration == hud.Frame().configuration, "paused settings preview blocked");
    ui.paused = false;

    hud.Alpha(Widget::Level, { 50.0f, std::nullopt });
    hud.Alpha(Widget::Level, { 80.0f, std::nullopt });
    AdvanceWidget(menu, Widget::Level, state, .016f, 16);
    Require(menu.uiMovie->alpha == 80, "latest alpha not coalesced");
    hud.Alpha(Widget::Level, { 0.0f, 100.0f });
    AdvanceWidget(menu, Widget::Level, state, .016f, 17);
    Require(menu.uiMovie->alpha == 80, "hide interrupted an unfinished fade");
    menu.uiMovie->alpha = 100;
    hud.Alpha(Widget::Level, { 0.0f, 100.0f });
    AdvanceWidget(menu, Widget::Level, state, .016f, 18);
    Require(menu.uiMovie->alpha == 0, "conditional hide ignored");
    menu.uiMovie->alphaIsNumber = false;
    menu.uiMovie->alpha = 100;
    hud.Alpha(Widget::Level, { 0.0f, 100.0f });
    AdvanceWidget(menu, Widget::Level, state, .016f, 19);
    Require(menu.uiMovie->alpha == 100, "non-numeric alpha read");

    // Eight concurrent producer threads, with reloads interleaved. Producers
    // must never touch the fake movie (thread affinity is checked on each call).
    std::atomic<bool> start = false;
    std::vector<std::thread> producers;
    for (std::size_t i = 0; i < WidgetCount; ++i) {
        producers.emplace_back([&, i] {
            while (!start.load()) { std::this_thread::yield(); }
            for (int j = 0; j < 20000; ++j) {
                hud.Alpha(static_cast<Widget>(i), { static_cast<float>(j % 100), std::nullopt });
                if (j % 127 == 0) { hud.Configure(); }
            }
        });
    }
    start = true;
    for (int j = 0; j < 1000; ++j) {
        if (j % 10 == 0) {
            hud.BeginSaveLoad(); hud.LoadingMenu(true); hud.FinishSaveLoad(true); hud.LoadingMenu(false);
        }
        AdvanceWidget(menu, Widget::Level, state, .016f, 20);
    }
    for (auto& producer : producers) { producer.join(); }
    hud.BeginSaveLoad(); hud.FinishSaveLoad(true);
    for (std::size_t i = 0; i < WidgetCount; ++i) {
        Require(!hud.TakeAlpha(static_cast<Widget>(i), hud.Frame().generation), "stress stale alpha");
    }
    Require(hud.Frame().ready, "stress left HUD blocked");
    std::cout << "PASS: " << checks << " assertions; 160000 concurrent requests; 100 reload cycles\n";
    return 0;
} catch (const std::exception& e) {
    std::cerr << "FAIL: " << e.what() << '\n';
    return 1;
}
