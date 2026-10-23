#pragma once
// Coalesces a settings request. Does not touch Scaleform on the caller's thread.
void CheckInI();
// UI-frame-only helper; never call from game/Papyrus/settings callbacks.
void SetHudVisible(RE::GFxMovieView* hud, const char* path, bool visible);
