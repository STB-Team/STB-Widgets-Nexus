#include "WidgetShout.h"
#include "Settings.h"
#include "HudRuntime.h"


WidgetShout::WidgetShout()
{
  const auto scale_form_manager = RE::BSScaleformManager::GetSingleton();

  logger::info("Construct menu");

  inputContext  = Context::kNone;
  depthPriority = 0;

  menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);
  menuFlags.set(RE::UI_MENU_FLAGS::kRequiresUpdate);
  menuFlags.set(RE::UI_MENU_FLAGS::kAllowSaving);

  if (uiMovie) {
    uiMovie->SetMouseCursorCount(0);
  }

  scale_form_manager->LoadMovieEx(this, MENU_PATH, [](RE::GFxMovieDef* def) -> void {
    def->SetState(RE::GFxState::StateType::kLog, RE::make_gptr<WidgetLogger>().get());
  });
}

auto WidgetShout::register_() -> void
{
  if (const auto ui = RE::UI::GetSingleton()) {
    ui->Register(MENU_NAME, creator);
    logger::info("Register menu");
    show();
  }
}

auto WidgetShout::show() -> void
{
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
	if (auto ui = RE::UI::GetSingleton(); ui && !ui->GetMenu(RE::BarterMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::ContainerMenu::MENU_NAME) && !ui->GetMenu(RE::TweenMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::MapMenu::MENU_NAME) && !ui->GetMenu(RE::InventoryMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::GiftMenu::MENU_NAME)) {
		logger::debug("Show menu");
		message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}
  }
}

auto WidgetShout::hide() -> void
{
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
    logger::debug("Hide menu");
    message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
  }
}


auto WidgetShout::update() -> void
{

  const auto ui = RE::UI::GetSingleton();
  if (!ui || ui->GameIsPaused()) {
    return;
  }

  const auto widgetShout = ui->GetMenu(MENU_NAME);
  if (!widgetShout || !widgetShout->uiMovie) {
    return;
  }

  const auto player = RE::PlayerCharacter::GetSingleton();
  if (!player) {
    return;
  }
  if (!ui->GetMenu<RE::HUDMenu>() || !ui->GetMenu<RE::HUDMenu>().get()->GetRuntimeData().shout)
	return;
 // std::string text = std::to_string(250.f - (ui->GetMenu<RE::HUDMenu>().get()->shout->fillPct * 2.5f));
  auto hp = 250.f - (ui->GetMenu<RE::HUDMenu>().get()->GetRuntimeData().shout->fillPct * 2.5f);
  if (hp > 250)
	hp = 250;
  else if (hp < 0)
	hp = 0;
  std::string result = std::to_string(hp);
  const RE::GFxValue state{ result };
  widgetShout->uiMovie->SetVariable("_root.shoutWidget.mask._width", state);
}

void WidgetShout::AdvanceMovie(const float interval, const uint32_t current_time)
{
    if (!STB::AdvanceWidget(*this, STB::Widget::Shout, frameState_, interval, current_time)) {
        return;
    }
    if (Settings::VisibleShout_ && frameState_.elapsed >= 0.0f) {
        update();
        frameState_.elapsed = 0.0f;
    }
}

auto WidgetShout::toggle_visibility(const bool mode) -> void
{
    STB::Hud().Visibility(mode);
}
