#include "WidgetGameTime.h"
#include "Settings.h"
#include "HudRuntime.h"


WidgetGameTime::WidgetGameTime()
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

auto WidgetGameTime::register_() -> void
{
  if (const auto ui = RE::UI::GetSingleton()) {
    ui->Register(MENU_NAME, creator);
    logger::info("Register menu");
    show();
  }
}

auto WidgetGameTime::show() -> void
{
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
	if (auto ui = RE::UI::GetSingleton(); ui && !ui->GetMenu(RE::TweenMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::ContainerMenu::MENU_NAME) && !ui->GetMenu(RE::MapMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::BarterMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::InventoryMenu::MENU_NAME) && !ui->GetMenu(RE::GiftMenu::MENU_NAME)) {
		logger::debug("Show menu");
		message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}
  }
}

auto WidgetGameTime::hide() -> void
{
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
    logger::debug("Hide menu");
    message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
  }
}

auto WidgetGameTime::update() -> void
{

  const auto ui = RE::UI::GetSingleton();
  if (!ui || ui->GameIsPaused()) {
    return;
  }

  const auto widgettime = ui->GetMenu(MENU_NAME);
  if (!widgettime || !widgettime->uiMovie) {
    return;
  }

  const auto player = RE::PlayerCharacter::GetSingleton();
  if (!player) {
    return;
  }

 //std::string number = "";
  double num = 24.00;
  if (auto cal = RE::Calendar::GetSingleton(); cal) {
	auto check = cal->GetCurrentGameTime();
	while (check >= 1.f)
          check -= 1.f;
	num = check * num;
  }
	//number = std::to_string(cal->GetCurrentGameTime() * num);
  int hour = static_cast<int>(num);
  int minute = static_cast<int>((num - hour) * 60);

  std::stringstream ss;
  ss << std::setfill('0') << std::setw(2) << hour << ":" << std::setfill('0') << std::setw(2) << minute;

  std::string timeStr = ss.str();
  //std::string result = number;

  const RE::GFxValue time{ timeStr };
  widgettime->uiMovie->Invoke("widget.Update", nullptr, &time, 1);
}

void WidgetGameTime::AdvanceMovie(const float interval, const uint32_t current_time)
{
    if (!STB::AdvanceWidget(*this, STB::Widget::GameTime, frameState_, interval, current_time)) {
        return;
    }
    if (Settings::VisibleGameTime_ && Settings::VisibleEquipKey && frameState_.elapsed >= 1.0f) {
        update();
        frameState_.elapsed = 0.0f;
    }
}

auto WidgetGameTime::toggle_visibility(const bool mode) -> void
{
    STB::Hud().Visibility(mode);
}
