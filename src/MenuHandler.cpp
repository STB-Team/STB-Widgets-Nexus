#include "MenuHandler.h"
#include "Settings.h"
#include "HudRuntime.h"
#include "WidgetManager.h"
#include "WidgetLvlWidget.h"
#include "WidgetGold.h"
#include "WidgetShout.h"
#include "WidgetWeight.h"
#include "WidgetEquip.h"
#include "WidgetResist.h"
#include "WidgetPlayTime.h"
#include "WidgetGameTime.h"

using namespace RE;
auto MenuHandler::get_singleton() noexcept -> MenuHandler*
{
    static MenuHandler instance;
    return std::addressof(instance);
}
auto MenuHandler::register_() -> void
{
    if (const auto ui = UI::GetSingleton()) {
        ui->AddEventSink(get_singleton());
    }
}
void check2(bool visible) { STB::Hud().Visibility(visible); }
void RequestHudReconfig() { CheckInI(); }

static void ShowAllWidgets()
{
    ResistWidget::show();
    WidgetGold::show();
    WidgetWeight::show();
    WidgetLvl::show();
    WidgetShout::show();
    WidgetPlayTime::show();
    WidgetGameTime::show();
    WidgetEquip::show();
}

auto MenuHandler::ProcessEvent(const MenuOpenCloseEvent* event,
    BSTEventSource<MenuOpenCloseEvent>*) -> BSEventNotifyControl
{
    if (!event) {
        return BSEventNotifyControl::kContinue;
    }
    if (event->menuName == LoadingMenu::MENU_NAME) {
        STB::Hud().LoadingMenu(event->opening);
        logger::info("Loading menu {}: HUD generation {}",
            event->opening ? "opened" : "closed", STB::Hud().Frame().generation);
        if (!event->opening) {
            ShowAllWidgets();  // queues UI messages; never closes an overlay
        }
    } else if (event->menuName == JournalMenu::MENU_NAME) {
        Settings::get_singleton().load(false);
        RequestHudReconfig();
    }
    // Visibility is decided from settled menu state during AdvanceMovie, not
    // inside a closing event or a game-thread task with stale menu state.
    return BSEventNotifyControl::kContinue;
}
