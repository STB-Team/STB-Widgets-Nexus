#include "WidgetManager.h"
#include "Settings.h"
#include "HudRuntime.h"

using namespace RE;

void SetHudVisible(RE::GFxMovieView* hud, const char* path, bool visible)
{
	if (!hud || !path) {
		return;
	}

	// No caching: another HUD mod can change the real Scaleform variable behind our
	// back, so a cached "already correct" value would leave the vanilla element in
	// the wrong state. Always write it - SetVariable on a bool is cheap.
	const RE::GFxValue value{ visible };
	hud->SetVariable(path, value);
}

void CheckInI()
{
    STB::Hud().Configure();
}

bool STB::ApplyWidgetConfiguration(Widget target, RE::GFxMovieView& movie)
{
    const auto& settings = Settings::get_singleton();
    const auto ui = UI::GetSingleton();
    const auto player = PlayerCharacter::GetSingleton();
    if (!ui || !player) {
        return false;
    }
	if (target == Widget::Resist) {
		const GFxValue preset = settings.PresetResist_;

		movie.Invoke("widget.setresistPreset", nullptr, &preset, 1);
		const GFxValue scalex = settings.scalexResist_;
		const GFxValue scale  = settings.scaleResist_;
		const GFxValue scaley = settings.scaleyResist_;
		GFxValue       Alpha  = settings.AlphaResist_;
		const GFxValue Visible = settings.VisibleResist_ && settings.VisibleEquipKey;
		const GFxValue Color   = settings.ColorResist_;
		movie.Invoke("widget.setresistColor", nullptr, &Color, 1);
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		if (settings.CombatResist_ && settings.VisibleResist_ && settings.VisibleResistKey &&
			!player->IsInCombat() && !settings.ChangeResist_)
			Alpha = 0;
		if (!settings.ChangeResist_ || settings.ChangeResistDelay > 0)
			movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
	}
	if (target == Widget::Level) {
		const GFxValue scalex  = settings.scalexLvl_;
		const GFxValue scale   = settings.scaleLvl_;
		const GFxValue scaley  = settings.scaleyLvl_;
		GFxValue       Alpha   = settings.AlphaLvl_;
		const GFxValue preset  = 4;

		const GFxValue Visible = settings.VisibleLvl_ && settings.VisibleEquipKey;
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setlvlPreset", nullptr, &preset, 1);
		const GFxValue expVisible = !settings.HideLvlNumbers_;
		movie.SetVariable("_root.widget.lvlExp_Text._visible", expVisible);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatLvl_ && settings.VisibleLvl_ && settings.VisibleLvlKey &&
			!player->IsInCombat() && !settings.ChangeLvl_)
			Alpha = 0;
		if (!settings.ChangeLvl_ || settings.ChangeLvlDelay > 0)
			movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (target == Widget::Equip) {
		const GFxValue scalex  = settings.scalexEquip_;
		const GFxValue scale   = settings.scaleEquip_;
		const GFxValue scaley  = settings.scaleyEquip_;
		GFxValue       Alpha   = settings.AlphaEquip_;
		const GFxValue preset  = settings.PresetEquip_;
		const GFxValue Visible = settings.VisibleEquip_ && settings.VisibleEquipKey;
		if (auto hud = ui->GetMenu<RE::HUDMenu>(); hud && hud->uiMovie) {
			// Only hide the vanilla arrow info when the STB equip widget is actually
			// shown (enabled + not toggled off by the hotkey). Otherwise we would leave
			// both hidden = the "missing HUD element" the bug report describes.
			const bool stbEquipShown = settings.VisibleEquip_ && settings.VisibleEquipKey;
			SetHudVisible(hud->uiMovie.get(), "_root.HUDMovieBaseInstance.ArrowInfoInstance._visible",
				!stbEquipShown);
		}
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setPreset", nullptr, &preset, 1);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatEquip_ && settings.VisibleEquip_ && settings.VisibleEquipKey &&
			!player->IsInCombat() && !settings.ChangeEquip_)
			Alpha = 0;
		if (!settings.ChangeEquip_ || settings.ChangeEquipDelay > 0)
			movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (target == Widget::Shout) {
		const GFxValue scalex  = settings.scalexShout_;
		const GFxValue scale   = settings.scaleShout_;
		const GFxValue scaley  = settings.scaleyShout_;
		const GFxValue Alpha   = settings.AlphaShout_;

		const GFxValue Visible = settings.VisibleShout_;
		if (auto hudMenu = ui->GetMenu<RE::HUDMenu>(); hudMenu && hudMenu->uiMovie) {
			SetHudVisible(hudMenu->uiMovie.get(),
				"_root.HUDMovieBaseInstance.CompassShoutMeterHolder.ShoutMeterInstance._visible",
				!settings.VisibleShout_);
		}

		movie.SetVariable("_root.shoutWidget._visible", Visible);
		movie.SetVariable("_root.shoutWidget._xscale", scale);
		movie.SetVariable("_root.shoutWidget._yscale", scale);
		movie.SetVariable("_root.shoutWidget._x", scalex);
		movie.SetVariable("_root.shoutWidget._y", scaley);
		movie.SetVariable("_root.shoutWidget._alpha", Alpha);
	}
	if (target == Widget::Gold) {
		const GFxValue scalex  = settings.scalexGold_;
		const GFxValue scale   = settings.scaleGold_;
		const GFxValue scaley  = settings.scaleyGold_;
		GFxValue       Alpha   = settings.AlphaGold_;

		const GFxValue Visible = settings.VisibleGold_ && settings.VisibleEquipKey;
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatGold_ && settings.VisibleGold_ && settings.VisibleGoldKey &&
			!player->IsInCombat() && !settings.ChangeGold_)
			Alpha = 0;
		if (!settings.ChangeGold_ || settings.ChangeGoldDelay > 0)
			movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (target == Widget::PlayTime) {
		const GFxValue scalex  = settings.scalexPlayTime_;
		const GFxValue scale   = settings.scalePlayTime_;
		const GFxValue scaley  = settings.scaleyPlayTime_;
		const GFxValue Alpha   = settings.AlphaPlayTime_;
		const GFxValue preset  = settings.PresetPlayTime_;

		const GFxValue Visible = settings.VisiblePlayTime_ && settings.VisibleEquipKey;
		movie.Invoke("widget.setPreset", nullptr, &preset, 1);
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
		movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (target == Widget::GameTime) {
		const GFxValue scalex  = settings.scalexGameTime_;
		const GFxValue scale   = settings.scaleGameTime_;
		const GFxValue scaley  = settings.scaleyGameTime_;
		const GFxValue Alpha   = settings.AlphaGameTime_;
		const GFxValue preset  = settings.PresetGameTime_;

		const GFxValue Visible = settings.VisibleGameTime_ && settings.VisibleEquipKey;
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setPreset", nullptr, &preset, 1);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
		movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (target == Widget::Weight) {
		const GFxValue scalex  = settings.scalexWeight_;
		const GFxValue scale   = settings.scaleWeight_;
		const GFxValue scaley  = settings.scaleyWeight_;
		GFxValue       Alpha   = settings.AlphaWeight_;

		const GFxValue Visible = settings.VisibleWeight_ && settings.VisibleEquipKey;
		movie.Invoke("widget.setVisible", nullptr, &Visible, 1);
		movie.Invoke("widget.setScale", nullptr, &scale, 1);
		movie.Invoke("widget.setPosX", nullptr, &scalex, 1);
		movie.Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatWeight_ && settings.VisibleWeight_ && settings.VisibleWeightKey &&
			!player->IsInCombat() && !settings.ChangeWeight_)
			Alpha = 0;
		if (!settings.ChangeWeight_ || settings.ChangeWeightDelay > 0)
			movie.Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
    return true;
}
