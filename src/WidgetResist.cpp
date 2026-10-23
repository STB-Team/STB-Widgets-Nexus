#include "WidgetResist.h"
#include "Settings.h"
#include "HudRuntime.h"

using namespace RE;

ResistWidget::ResistWidget() {
  const auto scale_form_manager = BSScaleformManager::GetSingleton();

  logger::info("Construct menu");
  depthPriority = 0;
  inputContext  = Context::kNone;
 
  menuFlags.set(UI_MENU_FLAGS::kAlwaysOpen);
  menuFlags.set(UI_MENU_FLAGS::kRequiresUpdate);
  menuFlags.set(UI_MENU_FLAGS::kAllowSaving);

  if (uiMovie) {
    uiMovie->SetMouseCursorCount(0);
  }

  scale_form_manager->LoadMovieEx(this, MENU_PATH, [](GFxMovieDef* def) -> void {
    def->SetState(GFxState::StateType::kLog, make_gptr<ResistLogger>().get());
  });
}

auto ResistWidget::register_() -> void {
  if (const auto ui = UI::GetSingleton()) {
    ui->Register(MENU_NAME, creator);
    logger::info("Register menu");
    show();
  }
}

auto ResistWidget::show() -> void {
  if (const auto message_queue = RE::UIMessageQueue::GetSingleton()) {
	if (auto ui = RE::UI::GetSingleton();
		!ui->GetMenu(RE::ContainerMenu::MENU_NAME) && !ui->GetMenu(RE::MapMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::BarterMenu::MENU_NAME) && !ui->GetMenu(RE::InventoryMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::GiftMenu::MENU_NAME) &&
										  !ui->GetMenu(RE::TweenMenu::MENU_NAME)) {
				logger::debug("Show menu");
				message_queue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
	}
  }
}

auto ResistWidget::hide() -> void {
  if (const auto message_queue = UIMessageQueue::GetSingleton()) {
    logger::debug("Hide menu");
    message_queue->AddMessage(MENU_NAME, UI_MESSAGE_TYPE::kHide, nullptr);
  }
}


auto ResistWidget::update() -> void {

  const auto ui = UI::GetSingleton();
  if (!ui || ui->GameIsPaused()) {
    return;
  }

  const auto resist_widget = ui->GetMenu(MENU_NAME);
  if (!resist_widget || !resist_widget->uiMovie) {
    return;
  }

  const auto player = PlayerCharacter::GetSingleton();
  if (!player) {
    return;
  }
  auto chel = player->AsActorValueOwner();
  int fire = (int)chel->GetActorValue(ActorValue::kResistFire);
  int frost = (int)chel->GetActorValue(ActorValue::kResistFrost);
  int shock = (int)chel->GetActorValue(ActorValue::kResistShock);
  int poison = (int)chel->GetActorValue(ActorValue::kPoisonResist);
  int magic = (int)chel->GetActorValue(ActorValue::kResistMagic);
  int disease = (int)chel->GetActorValue(ActorValue::kResistDisease);
  int armor = (int)chel->GetActorValue(ActorValue::kDamageResist);
  int speed = (int)chel->GetActorValue(ActorValue::kSpeedMult);

  const auto res =
	  fmt::format("{},{},{},{},{},{},{},{}"sv, fire,
		 frost, shock, magic, poison, disease, armor, speed);

  const GFxValue resists{ res };

 resist_widget->uiMovie->Invoke("widget.setresist_Text", nullptr, &resists, 1);
  
}

void ResistWidget::AdvanceMovie(const float interval, const uint32_t current_time)
{
    if (!STB::AdvanceWidget(*this, STB::Widget::Resist, frameState_, interval, current_time)) {
        return;
    }
    if (Settings::VisibleResist_ && Settings::VisibleEquipKey && frameState_.elapsed >= 0.2f) {
        update();
        frameState_.elapsed = 0.0f;
    }
}

auto ResistWidget::toggle_visibility(const bool mode) -> void
{
    STB::Hud().Visibility(mode);
}
