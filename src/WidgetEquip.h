#pragma once
#include "HudRequests.h"
RE::TESShout* GetShout(RE::Actor* a);
struct WidgetEquip final : RE::IMenu {
public:
  static constexpr const char* MENU_PATH = "equipWidget_STB";
  static constexpr const char* MENU_NAME = "equipWidget_STB";

  WidgetEquip();
  static inline RE::TESBoundObject* LastUseHP;
  static inline RE::TESBoundObject* LastUseST;
  static inline RE::TESBoundObject* LastUseMP;
  static auto register_() -> void;
  static auto show() -> void;
  static auto hide() -> void;
  static auto update() -> void;
  static auto toggle_visibility(bool mode) -> void;

  static auto creator() -> RE::stl::owner<RE::IMenu*> { return new WidgetEquip(); }

  auto AdvanceMovie(float interval, uint32_t current_time) -> void override;

private:
  STB::WidgetFrameState frameState_;
  class WidgetLogger final : public RE::GFxLog
  {
  public:
    void LogMessageVarg(LogMessageType, const char* fmt, const std::va_list arg_list) override {
      std::string format(fmt ? fmt : "");
      while (!format.empty() && format.back() == '\n') {
        format.pop_back();
      }

      std::va_list args;
      va_copy(args, arg_list);
      std::vector<char> buf(
          static_cast<std::size_t>(std::vsnprintf(0, 0, format.c_str(), arg_list) + 1));
      std::vsnprintf(buf.data(), buf.size(), format.c_str(), args);
      va_end(args);

      logger::info("{}"sv, buf.data());
    }
  };
};
