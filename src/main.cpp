#include "Settings.h"
#include "HudRuntime.h"
#include "WidgetEquip.h"
#include "WidgetLvlWidget.h"
#include "WidgetResist.h"
#include "WidgetWeight.h"
#include "WidgetGold.h"
#include "WidgetShout.h"
#include "MenuHandler.h"
#include "WidgetPlayTime.h"
#include "WidgetGameTime.h"
#include "WidgetManager.h"
#include "InputHandler.h"
#include "Hooks.h"
#include "Serialization.h"
#include "ImGuiMenu.h"
#include <Windows.h>

using namespace RE;

void MessageHandler(SKSE::MessagingInterface::Message* a_message)
{
	switch (a_message->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Input::InputEventHandler::Register();
		Settings::get_singleton().load(false);
		InstallHooks();
		SKSE_Menu::register_skse_menu();
		ResistWidget::register_();
		WidgetGold::register_();
		WidgetShout::register_();
		WidgetEquip::register_();
		WidgetWeight::register_();
		WidgetLvl::register_();
		MenuHandler::register_();
		WidgetPlayTime::register_();
		WidgetGameTime::register_();
		ResistWidget::show();
		WidgetGold::show();
		WidgetWeight::show();
		WidgetShout::show();
		WidgetEquip::show();
		WidgetLvl::show();
		WidgetPlayTime::show();
		WidgetGameTime::show();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		STB::Hud().BeginSaveLoad();
		logger::info("Save reload started: HUD generation {}", STB::Hud().Frame().generation);
		break;
	case SKSE::MessagingInterface::kNewGame:  // Player starts a new game from main menu.
		{
		Settings::get_singleton().load(false);
		STB::Hud().NewGame();
		ResistWidget::show();
		WidgetGold::show();
		WidgetWeight::show();
		WidgetShout::show();
		WidgetLvl::show();
		WidgetPlayTime::show();
		WidgetEquip::show();
		WidgetGameTime::show();
		break;
		}
	case SKSE::MessagingInterface::kPostLoadGame:  // Player's selected save game has finished loading.
		{
		Settings::get_singleton().load(false);
        const bool success = a_message->data != nullptr; // SKSE passes the bool as the pointer value
        STB::Hud().FinishSaveLoad(success);
        logger::info("Save reload finished: success={}, HUD generation {}", success, STB::Hud().Frame().generation);
		ResistWidget::show();
		WidgetGold::show();
		WidgetWeight::show();
		WidgetEquip::show();
		WidgetShout::show();
		WidgetLvl::show();
		WidgetPlayTime::show();
		WidgetGameTime::show();
		break;
		}
	}
	}
extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = "STB_Widgets";
	a_info->version = 1;

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;

	v.PluginVersion(1);
	v.PluginName("STB_Widgets");
	v.AuthorName("STB");
	v.UsesAddressLibrary(true);
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });
	v.HasNoStructUse(true);

	return v;
}();

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [thread %t] [%l] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	SKSE::Init(a_skse);
	logger::info("Save-reload UI ownership fix enabled (gameplay validation build)");
	SKSE::AllocTrampoline(1 << 9);

	SetupSerialization();
	auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
	return false;
	}
	
	return true;
}



