#include "WidgetLvlWidget.h"
#include "Settings.h"
#include "HudRuntime.h"


WidgetLvl::WidgetLvl()
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

auto WidgetLvl::register_() -> void
{
  if (const auto ui = RE::UI::GetSingleton()) {
    ui->Register(MENU_NAME, creator);
    logger::info("Register menu");
    show();
  }
}

auto WidgetLvl::show() -> void
{
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
	if (auto ui = RE::UI::GetSingleton(); ui && !ui->GetMenu(RE::TweenMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::BarterMenu::MENU_NAME) && !ui->GetMenu(RE::ContainerMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::MapMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::InventoryMenu::MENU_NAME) && !ui->GetMenu(RE::GiftMenu::MENU_NAME)) {
		logger::debug("Show menu");
		message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}
  }
}

auto WidgetLvl::hide() -> void
{
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
    logger::debug("Hide menu");
    message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
  }
}

float GetExperienceForLevel(int level)
{
  // fXPLevelUpMult * level + fXPLevelUpBase
  auto fXPLevelUpMult = 0.f;
  auto fXPLevelUpBase = 0.f;
  const auto settings = RE::GameSettingCollection::GetSingleton();
  if (settings) {
	auto levelUpBase = settings->GetSetting("fXPLevelUpBase");
	if (levelUpBase)
		fXPLevelUpBase = levelUpBase->GetFloat();
	auto levelUpMult = settings->GetSetting("fXPLevelUpMult");
	if (levelUpMult)
		fXPLevelUpMult = levelUpMult->GetFloat();
  }

  return fXPLevelUpBase + fXPLevelUpMult * level;
}
auto WidgetLvl::update() -> void
{

  const auto ui = RE::UI::GetSingleton();
  if (!ui || ui->GameIsPaused()) {
    return;
  }

  const auto widgetLvl = ui->GetMenu(MENU_NAME);
  if (!widgetLvl || !widgetLvl->uiMovie) {
    return;
  }

  const auto player = RE::PlayerCharacter::GetSingleton();
  if (!player || !player->GetInfoRuntimeData().skills || !player->GetInfoRuntimeData().skills->data) {
    return;
  }
  
  std::string result = std::to_string(player->GetInfoRuntimeData().skills->data->xp) + "|" +
	  std::to_string(GetExperienceForLevel(player->GetLevel())) + "|" + std::to_string(player->GetLevel()) + "|" +
	  std::to_string(player->IsInCombat());
  const RE::GFxValue lvl{ static_cast<RE::BSFixedString>(result) };
  widgetLvl->uiMovie->Invoke("widget.Update", nullptr, &lvl, 1);

  // Hide only the "current / max" XP number (lvlExp_Text), keeping the level and bar.
  // Re-applied every frame because the preset frame can re-instantiate the textfield
  // (which resets its _visible). The level text (lvl_Text) is left untouched.
  const RE::GFxValue expVisible{ !Settings::HideLvlNumbers_ };
  widgetLvl->uiMovie->SetVariable("_root.widget.lvlExp_Text._visible", expVisible);
}

void WidgetLvl::AdvanceMovie(const float interval, const uint32_t current_time)
{
    if (!STB::AdvanceWidget(*this, STB::Widget::Level, frameState_, interval, current_time)) {
        return;
    }
    if (Settings::VisibleLvl_ && Settings::VisibleEquipKey && frameState_.elapsed >= 0.2f) {
        update();
        frameState_.elapsed = 0.0f;
    }
}

auto WidgetLvl::toggle_visibility(const bool mode) -> void
{
    STB::Hud().Visibility(mode);
}
