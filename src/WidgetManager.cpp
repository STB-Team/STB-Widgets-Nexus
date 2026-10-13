#include "WidgetManager.h"
#include "Settings.h"

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

namespace
{
	// What we hide: the leaf pieces (icon, name, number) INSIDE the arrow counter.
	//
	// Do NOT touch these, all tried and all broke:
	//   ArrowInfoInstance._visible / _alpha  - other HUD code (ShowElements, fade managers)
	//       rewrites them every frame -> flicker.
	//   ArrowCountInstance(Alt)._visible     - scripting that clip takes it away from the
	//       ArrowInfoInstance fade timeline, so it stays stuck transparent -> counter gone.
	//
	// The leaves live in one-frame sprites with no timeline animation, and SkyHUD only ever
	// sets their text/position, never _visible, so nothing contends with us here. A path
	// that does not exist in this HUD (Alt pieces are SkyHUD-only) is skipped.
	struct ArrowPiece
	{
		const char* path;
		bool        hiddenByStb;  // we have forced this piece hidden
		bool        wasVisible;   // it was visible before we hid it (restored later)
	};

	ArrowPiece g_arrowPieces[] = {
		{ "_root.HUDMovieBaseInstance.ArrowInfoInstance.ArrowCountInstance.ArrowNumInstance._visible", false, false },
		{ "_root.HUDMovieBaseInstance.ArrowInfoInstance.ArrowCountInstance.ArrowInstance._visible", false, false },
		{ "_root.HUDMovieBaseInstance.ArrowInfoInstance.ArrowCountInstanceAlt.ArrowNameInstanceAlt._visible", false, false },
		{ "_root.HUDMovieBaseInstance.ArrowInfoInstance.ArrowCountInstanceAlt.ArrowNumInstanceAlt._visible", false, false },
		{ "_root.HUDMovieBaseInstance.ArrowInfoInstance.ArrowCountInstanceAlt.ArrowInstanceAlt._visible", false, false },
	};

	bool ReadBool(RE::GFxMovieView* a_movie, const char* a_path, bool& a_out)
	{
		RE::GFxValue value;
		if (!a_movie->GetVariable(&value, a_path) || !value.IsBool()) {
			return false;
		}
		a_out = value.GetBool();
		return true;
	}

	// Is STB's equipment panel actually on screen right now? Enabled + not hidden by the
	// hotkey + not faded out (combat mode / change mode fade it to alpha 0).
	bool StbEquipOnScreen(RE::GFxMovieView* a_stbMovie)
	{
		if (!Settings::VisibleEquip_ || !Settings::VisibleEquipKey) {
			return false;
		}
		if (!a_stbMovie) {
			return true;
		}
		RE::GFxValue alpha;
		if (a_stbMovie->GetVariable(&alpha, "_root.widget._alpha") && alpha.IsNumber()) {
			return alpha.GetNumber() > 0.0;
		}
		return true;
	}
}

void SyncVanillaArrowInfo(RE::GFxMovieView* hud, RE::GFxMovieView* stbEquipMovie)
{
	if (!hud) {
		return;
	}

	const bool hideHudCounter = StbEquipOnScreen(stbEquipMovie);

	for (auto& piece : g_arrowPieces) {
		bool visible = false;
		if (!ReadBool(hud, piece.path, visible)) {
			continue;  // this HUD has no such piece
		}

		if (hideHudCounter) {
			if (visible) {
				// Visible now: either not hidden yet, or the HUD movie was rebuilt.
				piece.wasVisible = true;
				hud->SetVariable(piece.path, RE::GFxValue{ false });
			}
			piece.hiddenByStb = true;
		} else if (piece.hiddenByStb) {
			if (piece.wasVisible && !visible) {
				hud->SetVariable(piece.path, RE::GFxValue{ true });
			}
			piece.hiddenByStb = false;
			piece.wasVisible  = false;
		}
	}
}

void CheckInI()
{
	const auto& settings = Settings::get_singleton();
	const auto  ui       = UI::GetSingleton();
	if (!ui) {
		return;
	}

	// The widget movies are mid-transition (torn down / rebuilt) only while a LoadingMenu
	// is active; invoking setPreset/gotoAndPlay on them then crashes Scaleform. The old
	// kHide-based flow was safe only because it CLOSED the menus during loads, so GetMenu()
	// returned null and these invokes were skipped. We keep the menus open now, so we skip
	// explicitly during loads. (Only LoadingMenu - not the post-load fade: the movies are
	// already settled during the fade, and configuring then is what avoids the ~1s pop.)
	if (ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
		return;
	}

	if (ui->GetMenu("resistWidget") && ui->GetMenu("resistWidget")->uiMovie) {
		const GFxValue preset = settings.PresetResist_;
		auto           widget = ui->GetMenu("resistWidget");
		widget->uiMovie->Invoke("widget.setresistPreset", nullptr, &preset, 1);
		const GFxValue scalex = settings.scalexResist_;
		const GFxValue scale  = settings.scaleResist_;
		const GFxValue scaley = settings.scaleyResist_;
		GFxValue       Alpha  = settings.AlphaResist_;
		const GFxValue Visible = settings.VisibleResist_;
		const GFxValue Color   = settings.ColorResist_;
		widget->uiMovie->Invoke("widget.setresistColor", nullptr, &Color, 1);
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		if (settings.CombatResist_ && settings.VisibleResist_ && settings.VisibleResistKey &&
			!PlayerCharacter::GetSingleton()->IsInCombat() && !settings.ChangeResist_)
			Alpha = 0;
		if (!settings.ChangeResist_ || settings.ChangeResistDelay > 0)
			widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
	}
	if (ui->GetMenu("lvlWidget") && ui->GetMenu("lvlWidget")->uiMovie) {
		const GFxValue scalex  = settings.scalexLvl_;
		const GFxValue scale   = settings.scaleLvl_;
		const GFxValue scaley  = settings.scaleyLvl_;
		GFxValue       Alpha   = settings.AlphaLvl_;
		const GFxValue preset  = 4;
		auto           widget  = ui->GetMenu("lvlWidget");
		const GFxValue Visible = settings.VisibleLvl_;
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setlvlPreset", nullptr, &preset, 1);
		const GFxValue expVisible = !settings.HideLvlNumbers_;
		widget->uiMovie->SetVariable("_root.widget.lvlExp_Text._visible", expVisible);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatLvl_ && settings.VisibleLvl_ && settings.VisibleLvlKey &&
			!PlayerCharacter::GetSingleton()->IsInCombat() && !settings.ChangeLvl_)
			Alpha = 0;
		if (!settings.ChangeLvl_ || settings.ChangeLvlDelay > 0)
			widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (auto widget = ui->GetMenu("equipWidget_STB"); widget && widget->uiMovie) {
		const GFxValue scalex  = settings.scalexEquip_;
		const GFxValue scale   = settings.scaleEquip_;
		const GFxValue scaley  = settings.scaleyEquip_;
		GFxValue       Alpha   = settings.AlphaEquip_;
		const GFxValue preset  = settings.PresetEquip_;
		const GFxValue Visible = settings.VisibleEquip_;
		if (auto hud = ui->GetMenu<RE::HUDMenu>(); hud && hud->uiMovie) {
			// Hide the HUD's own arrow counter only while the STB equip widget is actually
			// shown (enabled + not toggled off by the hotkey), and bring it back otherwise.
			// See SyncVanillaArrowInfo() for exactly what it hides and why.
			SyncVanillaArrowInfo(hud->uiMovie.get(), widget->uiMovie.get());
		}
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setPreset", nullptr, &preset, 1);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatEquip_ && settings.VisibleEquip_ && settings.VisibleEquipKey &&
			!PlayerCharacter::GetSingleton()->IsInCombat() && !settings.ChangeEquip_)
			Alpha = 0;
		if (!settings.ChangeEquip_ || settings.ChangeEquipDelay > 0)
			widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (ui->GetMenu("shoutWidget") && ui->GetMenu("shoutWidget")->uiMovie) {
		const GFxValue scalex  = settings.scalexShout_;
		const GFxValue scale   = settings.scaleShout_;
		const GFxValue scaley  = settings.scaleyShout_;
		const GFxValue Alpha   = settings.AlphaShout_;
		auto           widget  = ui->GetMenu("shoutWidget");
		const GFxValue Visible = settings.VisibleShout_;
		if (auto hudMenu = ui->GetMenu<RE::HUDMenu>(); hudMenu && hudMenu->uiMovie) {
			SetHudVisible(hudMenu->uiMovie.get(),
				"_root.HUDMovieBaseInstance.CompassShoutMeterHolder.ShoutMeterInstance._visible",
				!settings.VisibleShout_);
		}

		widget->uiMovie->SetVariable("_root.shoutWidget._visible", Visible);
		widget->uiMovie->SetVariable("_root.shoutWidget._xscale", scale);
		widget->uiMovie->SetVariable("_root.shoutWidget._yscale", scale);
		widget->uiMovie->SetVariable("_root.shoutWidget._x", scalex);
		widget->uiMovie->SetVariable("_root.shoutWidget._y", scaley);
		widget->uiMovie->SetVariable("_root.shoutWidget._alpha", Alpha);
	}
	if (ui->GetMenu("goldWidget") && ui->GetMenu("goldWidget")->uiMovie) {
		const GFxValue scalex  = settings.scalexGold_;
		const GFxValue scale   = settings.scaleGold_;
		const GFxValue scaley  = settings.scaleyGold_;
		GFxValue       Alpha   = settings.AlphaGold_;
		auto           widget  = ui->GetMenu("goldWidget");
		const GFxValue Visible = settings.VisibleGold_;
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatGold_ && settings.VisibleGold_ && settings.VisibleGoldKey &&
			!PlayerCharacter::GetSingleton()->IsInCombat() && !settings.ChangeGold_)
			Alpha = 0;
		if (!settings.ChangeGold_ || settings.ChangeGoldDelay > 0)
			widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (ui->GetMenu("playtimeWidget") && ui->GetMenu("playtimeWidget")->uiMovie) {
		const GFxValue scalex  = settings.scalexPlayTime_;
		const GFxValue scale   = settings.scalePlayTime_;
		const GFxValue scaley  = settings.scaleyPlayTime_;
		const GFxValue Alpha   = settings.AlphaPlayTime_;
		const GFxValue preset  = settings.PresetPlayTime_;
		auto           widget  = ui->GetMenu("playtimeWidget");
		const GFxValue Visible = settings.VisiblePlayTime_;
		widget->uiMovie->Invoke("widget.setPreset", nullptr, &preset, 1);
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
		widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (ui->GetMenu("gametimeWidget") && ui->GetMenu("gametimeWidget")->uiMovie) {
		const GFxValue scalex  = settings.scalexGameTime_;
		const GFxValue scale   = settings.scaleGameTime_;
		const GFxValue scaley  = settings.scaleyGameTime_;
		const GFxValue Alpha   = settings.AlphaGameTime_;
		const GFxValue preset  = settings.PresetGameTime_;
		auto           widget  = ui->GetMenu("gametimeWidget");
		const GFxValue Visible = settings.VisibleGameTime_;
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setPreset", nullptr, &preset, 1);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
		widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
	if (ui->GetMenu("weightWidget") && ui->GetMenu("weightWidget")->uiMovie) {
		const GFxValue scalex  = settings.scalexWeight_;
		const GFxValue scale   = settings.scaleWeight_;
		const GFxValue scaley  = settings.scaleyWeight_;
		GFxValue       Alpha   = settings.AlphaWeight_;
		auto           widget  = ui->GetMenu("weightWidget");
		const GFxValue Visible = settings.VisibleWeight_;
		if (settings.VisibleEquipKey)
			widget->uiMovie->Invoke("widget.setVisible", nullptr, &Visible, 1);
		widget->uiMovie->Invoke("widget.setScale", nullptr, &scale, 1);
		widget->uiMovie->Invoke("widget.setPosX", nullptr, &scalex, 1);
		widget->uiMovie->Invoke("widget.setPosY", nullptr, &scaley, 1);
		if (settings.CombatWeight_ && settings.VisibleWeight_ && settings.VisibleWeightKey &&
			!PlayerCharacter::GetSingleton()->IsInCombat() && !settings.ChangeWeight_)
			Alpha = 0;
		if (!settings.ChangeWeight_ || settings.ChangeWeightDelay > 0)
			widget->uiMovie->Invoke("widget.setAlpha", nullptr, &Alpha, 1);
	}
}
