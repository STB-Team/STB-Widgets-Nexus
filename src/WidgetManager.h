#pragma once

// Pushes the current settings (position/scale/alpha/visibility) into every
// loaded widget movie. Call after settings change or on menu transitions.
void CheckInI();

// Set the visibility of a variable inside the vanilla HUD movie. Writes
// unconditionally (no caching) so it stays correct even when another HUD mod
// changes the same Scaleform variable behind our back.
//
// Do NOT use this for _root.HUDMovieBaseInstance.ArrowInfoInstance: that clip is
// in the HUD's HudElements list, and HUDMenu.ShowElements() (vanilla and SkyHUD)
// rewrites its _visible on every HUD-mode change. Polling _visible=false against
// that produces the flicker. Use SyncVanillaArrowInfo() instead.
void SetHudVisible(RE::GFxMovieView* hud, const char* path, bool visible);

// Hides or restores the HUD's own arrow counter (vanilla or SkyHUD). It is hidden only
// while STB's equipment panel is actually on screen: enabled, not hidden by the hotkey,
// and not faded out by combat/change mode. Otherwise the HUD counter shows normally.
//
// Hides only the leaf pieces inside the counter (icon, name, number). Never touches
// ArrowInfoInstance or ArrowCountInstance(Alt): other HUD code drives those every frame
// or animates them, and forcing them causes flicker or leaves the counter stuck hidden.
// Reads before writing; only restores what it hid. stbEquipMovie may be null.
void SyncVanillaArrowInfo(RE::GFxMovieView* hud, RE::GFxMovieView* stbEquipMovie);
