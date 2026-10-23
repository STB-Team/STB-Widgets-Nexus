#include "Hooks.h"
#include "Settings.h"
#include "MenuHandler.h"
#include "WidgetEquip.h"
#include "HudRuntime.h"
#include <Windows.h>

using namespace RE;

void ShowWidget(bool hide)
{
    if (Settings::CombatEquip_ && Settings::VisibleEquip_) {
        STB::Hud().Alpha(STB::Widget::Equip, { hide ? 0.0f : Settings::AlphaEquip_, std::nullopt });
    }
    if (Settings::CombatLvl_ && Settings::VisibleLvl_) {
        STB::Hud().Alpha(STB::Widget::Level, { hide ? 0.0f : Settings::AlphaLvl_, std::nullopt });
    }
    if (Settings::CombatResist_ && Settings::VisibleResist_) {
        STB::Hud().Alpha(STB::Widget::Resist, { hide ? 0.0f : Settings::AlphaResist_, std::nullopt });
    }
    if (Settings::CombatGold_ && Settings::VisibleGold_) {
        STB::Hud().Alpha(STB::Widget::Gold, { hide ? 0.0f : Settings::AlphaGold_, std::nullopt });
    }
    if (Settings::CombatWeight_ && Settings::VisibleWeight_) {
        STB::Hud().Alpha(STB::Widget::Weight, { hide ? 0.0f : Settings::AlphaWeight_, std::nullopt });
    }
}
void HideEquip()
{
    if (Settings::VisibleEquip_) {
        STB::Hud().Alpha(STB::Widget::Equip, { 0.0f, Settings::AlphaEquip_ });
    }
}
void ShowWidgetEquip()
{
    if (Settings::ChangeEquip_ && Settings::VisibleEquip_) {
        if (Settings::ChangeEquipDelay <= 0) {
            STB::Hud().Alpha(STB::Widget::Equip, { Settings::AlphaEquip_, std::nullopt });
        }
        Settings::ChangeEquipDelay = Settings::ChangeEquipDelayBase;
    }
}
void HideResist()
{
    if (Settings::VisibleResist_) {
        STB::Hud().Alpha(STB::Widget::Resist, { 0.0f, Settings::AlphaResist_ });
    }
}
void ShowWidgetResist()
{
    if (Settings::ChangeResist_ && Settings::VisibleResist_) {
        if (Settings::ChangeResistDelay <= 0) {
            STB::Hud().Alpha(STB::Widget::Resist, { Settings::AlphaResist_, std::nullopt });
        }
        Settings::ChangeResistDelay = Settings::ChangeResistDelayBase;
    }
}
void HideWeight()
{
    if (Settings::VisibleWeight_) {
        STB::Hud().Alpha(STB::Widget::Weight, { 0.0f, Settings::AlphaWeight_ });
    }
}
void ShowWidgetWeight()
{
    if (Settings::ChangeWeight_ && Settings::VisibleWeight_) {
        if (Settings::ChangeWeightDelay <= 0) {
            STB::Hud().Alpha(STB::Widget::Weight, { Settings::AlphaWeight_, std::nullopt });
        }
        Settings::ChangeWeightDelay = Settings::ChangeWeightDelayBase;
    }
}
void HideGold()
{
    if (Settings::VisibleGold_) {
        STB::Hud().Alpha(STB::Widget::Gold, { 0.0f, Settings::AlphaGold_ });
    }
}
void ShowWidgetGold()
{
    if (Settings::ChangeGold_ && Settings::VisibleGold_) {
        if (Settings::ChangeGoldDelay <= 0) {
            STB::Hud().Alpha(STB::Widget::Gold, { Settings::AlphaGold_, std::nullopt });
        }
        Settings::ChangeGoldDelay = Settings::ChangeGoldDelayBase;
    }
}
void HideLvl()
{
    if (Settings::VisibleLvl_) {
        STB::Hud().Alpha(STB::Widget::Level, { 0.0f, Settings::AlphaLvl_ });
    }
}
void ShowWidgetLvl()
{
    if (Settings::ChangeLvl_ && Settings::VisibleLvl_) {
        if (Settings::ChangeLvlDelay <= 0) {
            STB::Hud().Alpha(STB::Widget::Level, { Settings::AlphaLvl_, std::nullopt });
        }
        Settings::ChangeLvlDelay = Settings::ChangeLvlDelayBase;
    }
}

class PlayerUpdate
{
public:
	static void Hook() { _Update = REL::Relocation<uintptr_t>(VTABLE_PlayerCharacter[0]).write_vfunc(0xad, Update);
		_AddItem = REL::Relocation<uintptr_t>(VTABLE_PlayerCharacter[0]).write_vfunc(0x5a, AddItem);
		_RemoveItem = REL::Relocation<uintptr_t>(VTABLE_PlayerCharacter[0]).write_vfunc(0x56, RemoveItem);
		_PickUpItem = REL::Relocation<uintptr_t>(VTABLE_PlayerCharacter[0]).write_vfunc(0xcc, PickUpItem);
		_UpdateRes02 = REL::Relocation<uintptr_t>(VTABLE_PlayerCharacter[5]).write_vfunc(0x5, UpdateRes02);
		_UpdateRes03 = REL::Relocation<uintptr_t>(VTABLE_PlayerCharacter[5]).write_vfunc(0x6, UpdateRes03);
		_ModValue = REL::Relocation<uintptr_t>(VTABLE_ValueModifierEffect[0]).write_vfunc(32, ModValue);
		_DualModValue = REL::Relocation<uintptr_t>(VTABLE_DualValueModifierEffect[0]).write_vfunc(32, DualModValue);
		_PeakModValue = REL::Relocation<uintptr_t>(VTABLE_PeakValueModifierEffect[0]).write_vfunc(32, PeakModValue);

	}

private:
	static void ModValue(ValueModifierEffect* a1, Actor* a2, float val, ActorValue avs)
	{
		if (val != 0 && a2->IsPlayerRef()) {
			auto av = a1->effect->baseEffect->data.primaryAV;
			if (av == ActorValue::kDamageResist || av == ActorValue::kPoisonResist || av == ActorValue::kResistFire ||
				av == ActorValue::kResistFrost || av == ActorValue::kSpeedMult || av == ActorValue::kResistShock ||
				av == ActorValue::kResistDisease || av == ActorValue::kResistMagic)
				ShowWidgetResist();
			if (av == ActorValue::kCarryWeight)
				ShowWidgetWeight();
		}
		return _ModValue(a1, a2, val, avs);
	}
	static inline REL::Relocation<decltype(ModValue)> _ModValue;
	static void DualModValue(DualValueModifierEffect* a1, Actor* a2, float val, ActorValue avs)
	{
		if (val != 0 && a2->IsPlayerRef()) {
			auto av = a1->effect->baseEffect->data.primaryAV;
			auto av2 = a1->effect->baseEffect->data.secondaryAV;
			if (av == ActorValue::kDamageResist || av == ActorValue::kPoisonResist || av == ActorValue::kResistFire ||
				av == ActorValue::kResistFrost || av == ActorValue::kSpeedMult || av == ActorValue::kResistShock ||
				av == ActorValue::kResistDisease || av == ActorValue::kResistMagic || av2 == ActorValue::kDamageResist ||
				av2 == ActorValue::kPoisonResist || av2 == ActorValue::kResistFire || av2 == ActorValue::kResistFrost ||
				av2 == ActorValue::kSpeedMult || av2 == ActorValue::kResistShock || av == ActorValue::kResistDisease ||
				av2 == ActorValue::kResistMagic)
				ShowWidgetResist();
			if (av == ActorValue::kCarryWeight)
				ShowWidgetWeight();
		}
		return _DualModValue(a1, a2, val, avs);
	}
	static inline REL::Relocation<decltype(DualModValue)> _DualModValue;
	static void PeakModValue(PeakValueModifierEffect* a1, Actor* a2, float val, ActorValue avs)
	{
		if (val != 0 && a2->IsPlayerRef()) {
			auto av = a1->effect->baseEffect->data.primaryAV;
			auto av2 = a1->effect->baseEffect->data.secondaryAV;
			if (av == ActorValue::kDamageResist || av == ActorValue::kPoisonResist || av == ActorValue::kResistFire ||
				av == ActorValue::kResistFrost || av == ActorValue::kSpeedMult || av == ActorValue::kResistShock ||
				av == ActorValue::kResistDisease || av == ActorValue::kResistMagic || av2 == ActorValue::kDamageResist ||
				av2 == ActorValue::kPoisonResist || av2 == ActorValue::kResistFire || av2 == ActorValue::kResistFrost ||
				av2 == ActorValue::kSpeedMult || av2 == ActorValue::kResistShock || av == ActorValue::kResistDisease ||
				av2 == ActorValue::kResistMagic)
				ShowWidgetResist();
			if (av == ActorValue::kCarryWeight)
				ShowWidgetWeight();
		}
		return _PeakModValue(a1, a2, val, avs);
	}
	static inline REL::Relocation<decltype(PeakModValue)> _PeakModValue;
	static void AddItem(Actor* this_, TESBoundObject* object, ExtraDataList* extra_list, int32_t count,
		TESObjectREFR* from_refr)
	{
		if (object->IsGold())
			ShowWidgetGold();
		if (object->GetWeight())
		ShowWidgetWeight();
		return _AddItem(this_, object, extra_list, count, from_refr);
	}
	static inline REL::Relocation<decltype(AddItem)> _AddItem;
	static ObjectRefHandle* RemoveItem(Actor* a, ObjectRefHandle* ref_handle, TESBoundObject* item, int count, int reason,
		ExtraDataList* extraList, TESObjectREFR* refr, void* a8, void* a9)
	{
		if (item->IsGold())
			ShowWidgetGold();
		if (item->GetWeight())
		ShowWidgetWeight();
		return _RemoveItem(a, ref_handle, item, count, reason, extraList, refr, a8, a9);
	}
	static inline REL::Relocation<decltype(RemoveItem)> _RemoveItem;
	static void PickUpItem(Actor* this_, TESObjectREFR* object, int32_t count, bool arg3, bool play_sound)
	{
		if (object->IsGold())
			ShowWidgetGold();
		if (object->GetWeight())
		ShowWidgetWeight();
		return _PickUpItem(this_, object, count, arg3, play_sound);
	}
	static inline REL::Relocation<decltype(PickUpItem)> _PickUpItem;
	static uint32_t UpdateRes02(ActorValueOwner* a1, ActorValue a2, float a3) {
		if (a3 != 0) {
			if (a2 == ActorValue::kDamageResist || a2 == ActorValue::kPoisonResist || a2 == ActorValue::kResistFire ||
				a2 == ActorValue::kResistFrost || a2 == ActorValue::kSpeedMult || a2 == ActorValue::kResistShock ||
				a2 == ActorValue::kResistDisease || a2 == ActorValue::kResistMagic)
				ShowWidgetResist();
			if (a2 == ActorValue::kCarryWeight)
				ShowWidgetWeight();
		}
		return _UpdateRes02(a1, a2, a3);
	}
	static inline REL::Relocation<decltype(UpdateRes02)> _UpdateRes02;
	static void UpdateRes03(ActorValueOwner* a1, ACTOR_VALUE_MODIFIER a2, ActorValue a3, float val)
	{
		if (val != 0) {
			if (a3 == ActorValue::kDamageResist || a3 == ActorValue::kPoisonResist || a3 == ActorValue::kResistFire ||
				a3 == ActorValue::kResistFrost || a3 == ActorValue::kSpeedMult || a3 == ActorValue::kResistShock ||
				a3 == ActorValue::kResistDisease || a3 == ActorValue::kResistMagic)
				ShowWidgetResist();
			if (a3 == ActorValue::kCarryWeight)
				ShowWidgetWeight();
		}
		return _UpdateRes03(a1, a2, a3, val);
	}
	static inline REL::Relocation<decltype(UpdateRes03)> _UpdateRes03;
	static void Update(PlayerCharacter* player, float delta)
	{
		if (auto camera = PlayerCamera::GetSingleton()){
			if (camera->currentState == camera->cameraStates[CameraState::kAutoVanity] ||
				camera->currentState == camera->cameraStates[CameraState::kVATS] ||
				camera->currentState == camera->cameraStates[CameraState::kFree])
				check2(false);
			else if (auto ui = UI::GetSingleton();
					 ui && !ui->IsMenuOpen(RE::MessageBoxMenu::MENU_NAME) && !ui->IsMenuOpen(RE::DialogueMenu::MENU_NAME) &&
					 !ui->IsMenuOpen(RE::CursorMenu::MENU_NAME) && !ui->IsMenuOpen(RE::MapMenu::MENU_NAME) &&
					 !ui->IsMenuOpen(RE::FaderMenu::MENU_NAME) && !ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME)) {
				// Configure widgets once their movies have loaded (coc / new game race),
				// then keep them visible in normal play.
				// Configuration is consumed by each widget on its UI frame.
				check2(true);
			}
			}
		const auto skills = player->GetInfoRuntimeData().skills;
		const float xp    = (skills && skills->data) ? skills->data->xp : Settings::LvlCheck[1];
		if ((!player->IsInCombat() || !Settings::CombatLvl_) &&
			(Settings::LvlCheck[0] != player->GetLevel() || Settings::LvlCheck[1] != xp)) {
			Settings::LvlCheck[0] = player->GetLevel();
			Settings::LvlCheck[1] = xp;
			ShowWidgetLvl();
		}
		if ((!player->IsInCombat() || !Settings::CombatEquip_) && Settings::ChangeEquip_) {
			if (auto ui = UI::GetSingleton(); ui && !ui->GetMenu<InventoryMenu>() && !ui->GetMenu<MagicMenu>())
				if (Settings::ChangeEquipDelay > 0)
			Settings::ChangeEquipDelay -= delta;
			if (Settings::ChangeEquipDelay <= 0)
				HideEquip();
		}
		if ((!player->IsInCombat() || !Settings::CombatResist_) && Settings::ChangeResist_) {
			if (auto ui = UI::GetSingleton(); ui && !ui->GetMenu<InventoryMenu>() && !ui->GetMenu<MagicMenu>())
				if (Settings::ChangeResistDelay > 0)
				Settings::ChangeResistDelay -= delta;
			if (Settings::ChangeResistDelay <= 0)
				HideResist();
		}
		if ((!player->IsInCombat() || !Settings::CombatWeight_) && Settings::ChangeWeight_) {
			if (auto ui = UI::GetSingleton(); ui && !ui->GetMenu<InventoryMenu>() && !ui->GetMenu<MagicMenu>())
				if (Settings::ChangeWeightDelay > 0)
				Settings::ChangeWeightDelay -= delta;
			if (Settings::ChangeWeightDelay <= 0)
				HideWeight();
		}
		if ((!player->IsInCombat() || !Settings::CombatGold_) && Settings::ChangeGold_) {
			if (auto ui = UI::GetSingleton(); ui && !ui->GetMenu<InventoryMenu>() && !ui->GetMenu<MagicMenu>())
				if (Settings::ChangeGoldDelay > 0)
				Settings::ChangeGoldDelay -= delta;
			if (Settings::ChangeGoldDelay <= 0)
				HideGold();
		}
		if ((!player->IsInCombat() || !Settings::CombatLvl_) && Settings::ChangeLvl_) {
			if (auto ui = UI::GetSingleton();
				ui && !ui->GetMenu<CraftingMenu>() && !ui->GetMenu<InventoryMenu>() && !ui->GetMenu<MagicMenu>())
				if (Settings::ChangeLvlDelay > 0)
				Settings::ChangeLvlDelay -= delta;
			if (Settings::ChangeLvlDelay <= 0)
				HideLvl();
		}
			if (player->IsInCombat()) {
				if (!Settings::isAtcive) {
					ShowWidget(false);
					Settings::isAtcive = !Settings::isAtcive;
				}
			} else if (Settings::isAtcive) {
			ShowWidget(true);
				Settings::isAtcive = !Settings::isAtcive;
			}
		_Update(player, delta);
	}

	static inline REL::Relocation<decltype(Update)> _Update;
};

class OnEquip
{
public:
	static void Hook()
	{
		_foo = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(37938, 38894).address() + RELOCATION_OFFSET(0xE5, 0x170), foo);
		_foo2 = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(37945, 38901).address() + RELOCATION_OFFSET(0x138, 0x1b9), foo2);
	}

private:
	static void foo2(ActorEquipManager* equip_manager, Actor* actor, TESBoundObject* bound_object, void* extra_data_list)
	{
		if (!equip_manager || !actor || !bound_object || !extra_data_list) {
				return _foo2(equip_manager, actor, bound_object, extra_data_list);
		}
		if (actor->IsPlayerRef()) {
				if (bound_object->As<TESObjectWEAP>() ||
					(bound_object->As<TESObjectARMO>() && bound_object->As<TESObjectARMO>()->IsShield()) ||
					bound_object->As<SpellItem>() || bound_object->As<TESShout>())
					ShowWidgetEquip();
		}
		return _foo2(equip_manager, actor, bound_object, extra_data_list);
	}

	static inline REL::Relocation<decltype(foo2)> _foo2;
	static void foo(ActorEquipManager* equip_manager, Actor* actor, TESBoundObject* bound_object, void* extra_data_list)
	{
		if (!equip_manager || !actor || !bound_object || !extra_data_list) {
				return _foo(equip_manager, actor, bound_object, extra_data_list);
		}
		if (actor->IsPlayerRef()) {
				if (bound_object->As<TESObjectWEAP>() || (bound_object->As<TESObjectARMO>() && bound_object->As<TESObjectARMO>()->IsShield()))
					ShowWidgetEquip();
				if (const auto alchemyItem = bound_object->As<AlchemyItem>()) {
					if (alchemyItem->IsPoison())
						ShowWidgetEquip();
					if (!alchemyItem->effects.empty() && alchemyItem->effects[0]) {
						if (auto ef = alchemyItem->effects[0]->baseEffect;
							ef && !ef->data.flags.any(EffectSetting::EffectSettingData::Flag::kRecover) &&
							!ef->data.flags.any(EffectSetting::EffectSettingData::Flag::kDetrimental) &&
							!ef->data.flags.any(EffectSetting::EffectSettingData::Flag::kHostile)) {
							if (ef->data.primaryAV == ActorValue::kHealth) {
								ShowWidgetEquip();
								WidgetEquip::LastUseHP = bound_object;
							} else if (ef->data.primaryAV == ActorValue::kStamina) {
								ShowWidgetEquip();
								WidgetEquip::LastUseST = bound_object;
							} else if (ef->data.primaryAV == ActorValue::kMagicka) {
								ShowWidgetEquip();
								WidgetEquip::LastUseMP = bound_object;
							}
						}
					}
				}
		}
				return _foo(equip_manager, actor, bound_object, extra_data_list);
	}

	static inline REL::Relocation<decltype(foo)> _foo;
};
class OnEquipSpell
{
public:
	static void Hook() { _foo = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(37939, 38895).address() + 0x47, foo); 
	_foo2 = SKSE::GetTrampoline().write_call<5>(
					RELOCATION_ID(50654, 51548).address() + RELOCATION_OFFSET(0x13a, 0x138), foo2);
	}

private:
	static void foo(void* a1, Actor* a, TESBoundObject* item, BGSEquipSlot** slot)
	{
				if (a->IsPlayerRef())
				ShowWidgetEquip();
				return _foo(a1, a, item, slot);
	}

	static inline REL::Relocation<decltype(foo)> _foo;
	static void foo2(void* a1, Actor* a2, MagicItem* a3)
	{
				ShowWidgetEquip();
				return _foo2(a1, a2, a3);
	}

	static inline REL::Relocation<decltype(foo2)> _foo2;
};

class OnEquipSpellUI
{
public:
	static void Hook()
	{ _foo = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(51164, 52044).address() + RELOCATION_OFFSET(0xff, 0x16b), foo);
				_foo2 = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(51164, 52044).address() + RELOCATION_OFFSET(0x11b, 0x187),
					foo2);
	}

private:
	static void foo(void* a1, PlayerCharacter* a2, SpellItem* a3, BGSEquipSlot* a4)
	{
				ShowWidgetEquip();

				return _foo(a1, a2, a3, a4);
	}

	static inline REL::Relocation<decltype(foo)> _foo;
	static void foo2(void* a1, Actor* a2, MagicItem* a3)
	{
				ShowWidgetEquip();
				return _foo2(a1, a2, a3);
	}

	static inline REL::Relocation<decltype(foo2)> _foo2;
};

class HideShout
{
public:
	static void Hook()
	{
		_foo = SKSE::GetTrampoline().write_call<5>(RELOCATION_ID(50768, 51663).address() + 0x5d, foo);
	}

private:
	static char foo(GFxValue::ObjectInterface* a1, void* data, GFxValue* result, char* name, GFxValue* args, int32_t numArgs,
		char isDObj)
	{
		if (Settings::VisibleShout_)
		return false;
		return _foo(a1, data, result, name, args, numArgs, isDObj);
	}

	static inline REL::Relocation<decltype(foo)> _foo;
};

void InstallHooks()
{
	PlayerUpdate::Hook();
	OnEquip::Hook();
	OnEquipSpell::Hook();
	OnEquipSpellUI::Hook();
	HideShout::Hook(); 
}
