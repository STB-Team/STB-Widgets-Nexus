#pragma once
#include "HudRequests.h"

namespace STB
{
HudRequests& Hud();
bool HudBlocked(RE::UI* ui);
// Called only by the owning menu's AdvanceMovie override. No engine calls are
// made under the request mutex; loading notifications cannot deadlock the UI.
bool AdvanceWidget(RE::IMenu& menu, Widget widget, WidgetFrameState& state,
    float interval, std::uint32_t currentTime);
bool ApplyWidgetConfiguration(Widget widget, RE::GFxMovieView& movie);
}
