#include "HudRuntime.h"

namespace STB
{
HudRequests& Hud()
{
    static HudRequests requests;
    return requests;
}

bool HudBlocked(RE::UI* ui)
{
    if (!ui) {
        return true;
    }
    constexpr std::string_view menus[] = {
        RE::LoadingMenu::MENU_NAME, RE::FaderMenu::MENU_NAME, RE::MainMenu::MENU_NAME,
        RE::InventoryMenu::MENU_NAME, RE::CraftingMenu::MENU_NAME, RE::BarterMenu::MENU_NAME,
        RE::TweenMenu::MENU_NAME, RE::GiftMenu::MENU_NAME, RE::ContainerMenu::MENU_NAME,
        RE::MagicMenu::MENU_NAME, RE::JournalMenu::MENU_NAME, RE::LockpickingMenu::MENU_NAME,
        RE::RaceSexMenu::MENU_NAME, RE::MapMenu::MENU_NAME, RE::DialogueMenu::MENU_NAME,
        RE::CursorMenu::MENU_NAME, RE::MessageBoxMenu::MENU_NAME
    };
    for (auto name : menus) {
        if (ui->IsMenuOpen(name)) {
            return true;
        }
    }
    return false;
}

bool AdvanceWidget(RE::IMenu& menu, Widget widget, WidgetFrameState& state,
    float interval, std::uint32_t currentTime)
{
    if (!menu.uiMovie) {
        return false;
    }
    auto ui = RE::UI::GetSingleton();
    const auto before = Hud().Frame();
    if (!ui || !before.ready || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
        menu.uiMovie->SetVisible(false);
        return false;
    }

    // Finish this movie's advance before any setPreset/gotoAndPlay or updates.
    // Do not configure other widgets: a delayed/new movie needs its own advance.
    menu.IMenu::AdvanceMovie(interval, currentTime);
    const auto frame = Hud().Frame();
    if (!frame.ready || frame.generation != before.generation) {
        return false;
    }
    if (state.configuration != frame.configuration) {
        if (!ApplyWidgetConfiguration(widget, *menu.uiMovie)) {
            return false;
        }
        state.configuration = frame.configuration;
        state.elapsed = 100.0f;
        logger::info("HUD widget {} configured: generation {}, revision {}",
            static_cast<std::size_t>(widget), frame.generation, frame.configuration);
    }
    if (const auto alpha = Hud().TakeAlpha(widget, frame.generation)) {
        RE::GFxValue current;
        const bool apply = !alpha->minimumCurrent ||
            (menu.uiMovie->GetVariable(&current, "_root.widget._alpha") &&
                current.IsNumber() && current.GetNumber() >= *alpha->minimumCurrent);
        if (apply) {
            const RE::GFxValue value{ alpha->value };
            menu.uiMovie->Invoke("widget.tweenAlpha", nullptr, &value, 1);
        }
    }
    menu.uiMovie->SetVisible(frame.visible && !HudBlocked(ui));
    if (ui->GameIsPaused()) {
        return false;
    }
    state.elapsed += (std::max)(0.0f, interval);
    return true;
}
}
