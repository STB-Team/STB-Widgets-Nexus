#include "WidgetEquip.h"
#include "Settings.h"
#include "HudRuntime.h"
#include "WidgetManager.h"  // SetHudVisible

using namespace RE;

// Magic forms coming from other mods can have an empty effect list; reading
// effects[0] on those would be an out-of-bounds access (crash). These helpers
// return a safe default instead.
static RE::ActorValue SafeAssociatedSkill(RE::SpellItem* spell)
{
  if (spell && !spell->effects.empty() && spell->effects[0] && spell->effects[0]->baseEffect) {
    return spell->effects[0]->baseEffect->data.associatedSkill;
  }
  return RE::ActorValue::kNone;
}

static bool HasFirstEffect(RE::MagicItem* item)
{
  return item && !item->effects.empty() && item->effects[0] && item->effects[0]->baseEffect;
}

WidgetEquip::WidgetEquip()
{
  const auto scale_form_manager = BSScaleformManager::GetSingleton();

  logger::info("Construct menu");

  inputContext  = Context::kNone;
  depthPriority = 0;

  menuFlags.set(UI_MENU_FLAGS::kAlwaysOpen);
  menuFlags.set(UI_MENU_FLAGS::kRequiresUpdate);
  menuFlags.set(UI_MENU_FLAGS::kAllowSaving);

  if (uiMovie) {
    uiMovie->SetMouseCursorCount(0);
  }

  scale_form_manager->LoadMovieEx(this, MENU_PATH, [](GFxMovieDef* def) -> void {
    def->SetState(GFxState::StateType::kLog, make_gptr<WidgetLogger>().get());
  });
}

auto WidgetEquip::register_() -> void
{
  if (const auto ui = UI::GetSingleton()) {
    ui->Register(MENU_NAME, creator);
    logger::info("Register menu");
    show();
  }
}

auto WidgetEquip::show() -> void
{
  if (const auto message_queue = UIMessageQueue::GetSingleton()) {
	if (auto ui = UI::GetSingleton(); !ui->GetMenu(RE::TweenMenu::MENU_NAME) &&
									  !ui->GetMenu(RE::BarterMenu::MENU_NAME) && !ui->GetMenu(ContainerMenu::MENU_NAME) &&
									  !ui->GetMenu(MapMenu::MENU_NAME) &&
									  !ui->GetMenu(InventoryMenu::MENU_NAME) && !ui->GetMenu(GiftMenu::MENU_NAME)) {
		logger::debug("Show menu");
		message_queue->AddMessage(MENU_NAME, UI_MESSAGE_TYPE::kShow, nullptr);
	}
  }
}

auto WidgetEquip::hide() -> void
{
  if (const auto message_queue = UIMessageQueue::GetSingleton()) {
    logger::debug("Hide menu");
    message_queue->AddMessage(MENU_NAME, UI_MESSAGE_TYPE::kHide, nullptr);
  }
}

bool IsWordUnlocked(int a1, int a2, int a3, TESWordOfPower* word)
{
  using func_t = decltype(IsWordUnlocked);
  REL::Relocation<func_t> func{ RELOCATION_ID(54869, 55502) };
  return func(a1, a2, a3, word);
}

int GetFormIndex(TESForm* form) {
  //if (form->formID == 0x1F4)
//	return 0;
  if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeClaw")) 
	  return 2;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeSpear"))
	return 7;
  else if(form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeDagger"))
	return 1;

  else if (form->As<TESObjectWEAP>() &&
		   form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeKatana") && form->As<TESObjectWEAP>()->GetWeaponType() != WeaponTypes::kTwoHandSword)
	return 3;

  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeSword"))
	return 4;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeWarAxe"))
	return 5;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeMace"))
	return 6;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeKatana") &&
		   form->As<TESObjectWEAP>()->GetWeaponType() == WeaponTypes::kTwoHandSword)
	return 8;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeGreatsword"))
	return 9;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeBattleaxe"))
	return 10;
  else if (form->As<TESObjectWEAP>() && (form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeQuarterstaff") ||
											form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeQtrStaff")))
	return 14;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeWarhammer") &&
		   !form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeQuarterstaff"))
	return 11;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->HasKeywordString("WeapTypeGiantGreatsword"))
	return 13;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->GetWeaponType() == WEAPON_TYPE::kBow)
	return 15;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->GetWeaponType() == WEAPON_TYPE::kCrossbow)
	return 17;
  else if (form->As<TESObjectARMO>() && form->As<TESObjectARMO>()->IsLightArmor())
	return 19;
  else if (form->As<TESObjectARMO>() && form->As<TESObjectARMO>()->IsHeavyArmor())
	return 20;
  else if (SafeAssociatedSkill(form->As<SpellItem>()) == ActorValue::kAlteration)
	return 21;
  else if (SafeAssociatedSkill(form->As<SpellItem>()) == ActorValue::kConjuration)
	return 22;
  else if (SafeAssociatedSkill(form->As<SpellItem>()) == ActorValue::kDestruction)
	return 23;
  else if (SafeAssociatedSkill(form->As<SpellItem>()) == ActorValue::kIllusion)
	return 24;
  else if (SafeAssociatedSkill(form->As<SpellItem>()) == ActorValue::kRestoration)
	return 25;
  else if (form->As<ScrollItem>())
	return 26;
  else if (form->As<TESObjectWEAP>() && form->As<TESObjectWEAP>()->GetWeaponType() == WeaponTypes::kStaff)
	return 27;
  return 0;
}
EnchantmentItem* CheckEnch(Actor* a, bool left)
{
  if (!a) {
	return nullptr;
  }

  const auto equippedObject = a->GetEquippedObject(left);
  const auto weapon = equippedObject ? equippedObject->As<TESObjectWEAP>() : nullptr;
  if (!weapon) {
	return nullptr;
  }

  // Only inspect the entry that is actually equipped in this hand. The old code
  // rebuilt and scanned the actor's complete weapon inventory every update, which
  // could race an inventory mutation (for example, crafting a weapon) and also
  // generated needless allocator traffic on the game's update thread.
  if (const auto entry = a->GetEquippedEntryData(left); entry && entry->extraLists) {
	for (const auto& extraList : *entry->extraLists) {
	  if (!extraList) {
		continue;
	  }

	  const bool wornInThisHand = left ? extraList->GetByType<ExtraWornLeft>() != nullptr
	                                   : extraList->GetByType<ExtraWorn>() != nullptr;
	  if (!wornInThisHand) {
		continue;
	  }

	  if (const auto extraEnchantment = extraList->GetByType<ExtraEnchantment>();
	      extraEnchantment && extraEnchantment->enchantment) {
		return extraEnchantment->enchantment;
	  }
	}
  }

  if (weapon->formEnchanting) {
	return weapon->formEnchanting;
  }
  return nullptr;
}

ExtraPoison* get_equipped_weapon_poison_data(const Actor* a_actor, bool a_leftHand)
{
  if (const auto equippedEntryData = a_actor->GetEquippedEntryData(a_leftHand); equippedEntryData) {
	if (equippedEntryData->extraLists) {
		for (const auto& xList : *equippedEntryData->extraLists) {
			if (xList) {
				if (const auto xPoison = xList->GetByType<ExtraPoison>(); xPoison) {
					return xPoison;
				}
			}
		}
	}
  }

  return nullptr;
}

std::uint32_t GetEquippedWeaponPoisonCount(const Actor* a_actor, bool a_leftHand)
{
  const auto xPoison = get_equipped_weapon_poison_data(a_actor, a_leftHand);
  return xPoison ? xPoison->count : 0;
}
bool WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword)
{
  if (actor && keyword) {
	auto inv = actor->GetInventoryChanges();
	if (!inv) {
		return false;
	}
	using FuncT = bool (*)(RE::InventoryChanges*, RE::BGSKeyword*);
	const REL::Relocation<FuncT> func{ RELOCATION_ID(15808, 16046) };
	return func(inv, keyword);
  }
  return false;
}
bool TESObjectREFR__HasEffectKeyword(RE::TESObjectREFR* a, RE::BGSKeyword* kwd)
{
  using func_t = decltype(TESObjectREFR__HasEffectKeyword);
  REL::Relocation<func_t> func{ RELOCATION_ID(19220, 19646) };
  return func(a, kwd);
}
bool HasKeywordAll(RE::Actor* actor, RE::BGSKeyword* keyword)
{
  const auto result =
	  actor->HasKeyword(keyword) || TESObjectREFR__HasEffectKeyword(actor, keyword) || WornHasKeyword(actor, keyword);
  return result;
}

RE::AlchemyItem* GetPoisonOnWeap(RE::InventoryEntryData* entry)
{
  if (entry && entry->extraLists)
	for (const auto& xList : *entry->extraLists)
		if (xList)
			if (const auto xPoison = xList->GetByType<RE::ExtraPoison>(); xPoison)
				return xPoison->poison;
  return nullptr;
}
std::string MakeWeaponInfo(PlayerCharacter* player, bool left) {
  std::string w = "&";
  auto num = 0.f;
	const auto equippedObject = player ? player->GetEquippedObject(left) : nullptr;
	const auto weap = equippedObject ? equippedObject->As<TESObjectWEAP>() : nullptr;
	if (!weap) {
		return {};
	}
	const auto equippedEntry = player->GetEquippedEntryData(left);
	if (player->GetCurrentAmmo() && weap->HasKeywordString("WeapTypeBow")) {
		auto scale = 1.f;
		BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModAttackDamage, player, nullptr, nullptr, &scale);
		num = player->GetCurrentAmmo()->data.damage * scale;
	}
	if (equippedEntry) {
		num += player->GetDamage(equippedEntry);
	}
	std::string damage = std::to_string((int)ceil(num));
  if (auto ench = CheckEnch(player, left); HasFirstEffect(ench)) {
	std::string color = "0";
	if (ench->effects[0]->baseEffect->HasKeywordString("MagicDamageFire"))
		color = "<font color='#e12f2f'>";
	else if (ench->effects[0]->baseEffect->HasKeywordString("MagicDamageFrost"))
		color = "<font color='#1ffbff'>";
	else if (ench->effects[0]->baseEffect->HasKeywordString("MagicDamageShock"))
		color = "<font color='#961696'>";
	auto scale = 1.f;
	BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, player, ench, nullptr, &scale);
	if (color != "0")
	damage = damage + "+" + color + std::to_string((int)ceil(ench->effects[0]->effectItem.magnitude * scale)) + "</font>";
	else
	damage = damage + "+" + std::to_string((int)ceil(ench->effects[0]->effectItem.magnitude * scale));
  } 
  std::string poisonnum = "0";
  std::string poisonDMG = "0";
  if (GetEquippedWeaponPoisonCount(player, left) > 0)
	poisonnum = std::to_string(GetEquippedWeaponPoisonCount(player, left));
	if (auto pois = GetPoisonOnWeap(equippedEntry); HasFirstEffect(pois)) {
	auto scale = 1.f;
	BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, player, pois, nullptr, &scale);
	poisonDMG = std::to_string((int)ceil(pois->effects[0]->effectItem.magnitude * scale));
  }
  auto name = weap->GetName();
	if (equippedEntry && equippedEntry->GetDisplayName())
		name = equippedEntry->GetDisplayName();
	int index = GetFormIndex(equippedObject);
  if (weap->formID == 0x1f4)
  name = "";
  std::string result =
	  name + w +
	                   std::to_string(index) +
	                   w + damage + w +
	                   poisonnum + w + poisonDMG;
 return result ;
}
//float GetArmorValue(PlayerCharacter* player, InventoryEntryData* a2)
//{
// using func_t = decltype(GetArmorValue);
// REL::Relocation<func_t> func{ REL::ID(39175) };
// return func(player, a2);
//}
RE::InventoryEntryData* CheckForm(RE::Actor* a, TESBoundObject* item)
{
 std::set<RE::TESBoundObject*> inv;
 auto changes = a->GetInventoryChanges();
 if (changes && changes->entryList) {
	for (auto entry : *changes->entryList) {
	if (entry && entry->object && item == entry->object) {
			inv.insert(entry->object);
			return entry;
	}
  }
 }
 return nullptr;
}

RE::TESShout* GetShout(RE::Actor* a)
{
 using func_t = decltype(GetShout);
 REL::Relocation<func_t> func{ RELOCATION_ID(37822, 38771) };
 return func(a);
}

auto WidgetEquip::update() -> void
{

  const auto ui = UI::GetSingleton();
  if (!ui || ui->GameIsPaused()) {
    return;
  }

  const auto widgetweight = ui->GetMenu(MENU_NAME);
  if (!widgetweight || !widgetweight->uiMovie) {
    return;
  }

  const auto player = PlayerCharacter::GetSingleton();
  if (!player) {
    return;
  }
 std::string w = "&";
 std::string arrow;
 std::string shout;
 std::string left;
 std::string right;
 std::string twohand;
 std::string pot;
 std::string zero = "0";
 auto leftobj = player->GetEquippedObject(true);
 auto rightobj = player->GetEquippedObject(false);
 if (!rightobj) {
	auto scale = player->AsActorValueOwner()->GetActorValue(ActorValue::kUnarmedDamage) *
		         player->AsActorValueOwner()->GetActorValue(ActorValue::kAttackDamageMult);
	BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModAttackDamage, player, nullptr, nullptr, &scale);
	right = "" + w + std::to_string(0) + w + std::to_string((int)ceil(scale)) + w + "" + w + "";
 }
 if (!leftobj && !rightobj) {
	auto scale = player->AsActorValueOwner()->GetActorValue(ActorValue::kUnarmedDamage) *
		         player->AsActorValueOwner()->GetActorValue(ActorValue::kAttackDamageMult);
	BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModAttackDamage, player, nullptr, nullptr, &scale);
	left = "" + w + std::to_string(0) + w + std::to_string((int)ceil(scale)) + w + "" + w + "";
 }
 if (rightobj && rightobj->As<TESObjectWEAP>()) {
	if (auto weap = rightobj->As<TESObjectWEAP>()) {
	if ((weap->GetWeaponType() == WeaponTypes::kCrossbow || weap->GetWeaponType() == WeaponTypes::kTwoHandAxe ||
			weap->GetWeaponType() == WeaponTypes::kBow || weap->GetWeaponType() == WeaponTypes::kTwoHandSword) &&
		(leftobj == rightobj)) {
			twohand = MakeWeaponInfo(player, false);
	} else if (weap->GetWeaponType() != WeaponTypes::kStaff) {
			right = MakeWeaponInfo(player, false);
	} else {
			auto name = rightobj->GetName();
			if (player->GetEquippedEntryData(false) && player->GetEquippedEntryData(false)->GetDisplayName())
				name = player->GetEquippedEntryData(false)->GetDisplayName();
			auto mag = 0.f;
			if (auto ench = CheckEnch(player, false); HasFirstEffect(ench))
				mag = ench->effects[0]->effectItem.magnitude;
			right = name + w + std::to_string(GetFormIndex(rightobj)) + w +
				    std::to_string((int)ceil(mag)) + w + zero +
				    w + zero;
	}
	}
 }
 if (leftobj && leftobj->As<TESObjectWEAP>()) {
	if (auto weap = leftobj->As<TESObjectWEAP>();
		(weap->GetWeaponType() != WeaponTypes::kCrossbow && weap->GetWeaponType() != WeaponTypes::kBow &&
			player->GetEquippedEntryData(true) && player->GetEquippedEntryData(false) != player->GetEquippedEntryData(true)))
	if (weap->GetWeaponType() != WeaponTypes::kStaff) {
			left = MakeWeaponInfo(player, true);
	} else {
			auto name = leftobj->GetName();
			if (player->GetEquippedEntryData(true) && player->GetEquippedEntryData(true)->GetDisplayName())
				name = player->GetEquippedEntryData(true)->GetDisplayName();
			auto mag = 0.f;
			if (auto ench = CheckEnch(player, true); HasFirstEffect(ench))
				mag = ench->effects[0]->effectItem.magnitude;
			left = name + w + std::to_string(GetFormIndex(leftobj)) + w +
				   std::to_string((int)ceil(mag)) + w + zero +
				   w + zero;
	}
 }
 if (leftobj && leftobj->As<ScrollItem>())
	left = leftobj->GetName() + w + std::to_string(GetFormIndex(leftobj)) + w + zero + w + zero + w + zero;
 if (rightobj && rightobj->As<ScrollItem>())
	right = rightobj->GetName() + w + std::to_string(GetFormIndex(rightobj)) + w + zero + w + zero + w + zero;

   if (leftobj && leftobj->As<TESObjectARMO>() && leftobj->As<TESObjectARMO>()->IsShield() && CheckForm(player, leftobj->As<TESBoundObject>())) {
	//auto scale = 1.f;
	//BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModArmorRating, player, leftobj->As<TESObjectARMO>(),
	//	&scale);
	//auto blockarmor = (ceil(leftobj->As<TESObjectARMO>()->armorRating / 100.f)) * scale;
	auto name = leftobj->GetName();
	if (player->GetEquippedEntryData(true) && player->GetEquippedEntryData(true)->GetDisplayName())
	name = player->GetEquippedEntryData(true)->GetDisplayName();
	int block = (int)ceil(
		player->GetArmorValue(CheckForm(player, leftobj->As<TESBoundObject>())));  // (int)ceil(blockarmor);
	left = name + w + std::to_string(GetFormIndex(leftobj)) + w +
		   std::to_string(block);
	}
 if (leftobj && HasFirstEffect(leftobj->As<SpellItem>())) {
	auto scale = 1.f;
	BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, player, leftobj->As<SpellItem>(), nullptr,
		&scale);
	std::string DMG = std::to_string((int)ceil((leftobj->As<SpellItem>()->effects[0]->effectItem.magnitude * scale)));
	std::string color;
	if (leftobj->As<SpellItem>()->effects[0]->baseEffect->HasKeywordString("MagicDamageFire"))
		color = "<font color='#e12f2f'>";
	else if (leftobj->As<SpellItem>()->effects[0]->baseEffect->HasKeywordString("MagicDamageFrost"))
		color = "<font color='#1ffbff'>";
	else if (leftobj->As<SpellItem>()->effects[0]->baseEffect->HasKeywordString("MagicDamageShock"))
		color = "<font color='#961696'>";
	DMG = color + DMG + "</font>";
	left = leftobj->GetName() + w + std::to_string(GetFormIndex(leftobj)) + w + DMG + w + zero + w + zero;
 }
 if (rightobj && HasFirstEffect(rightobj->As<SpellItem>())) {
	auto scale = 1.f;
	BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, player, rightobj->As<SpellItem>(), nullptr,
		&scale);
	std::string DMG = std::to_string((int)ceil((rightobj->As<SpellItem>()->effects[0]->effectItem.magnitude * scale)));
	std::string color;
	if (rightobj->As<SpellItem>()->effects[0]->baseEffect->HasKeywordString("MagicDamageFire"))
		color = "<font color='#e12f2f'>";
	else if (rightobj->As<SpellItem>()->effects[0]->baseEffect->HasKeywordString("MagicDamageFrost"))
		color = "<font color='#1ffbff'>";
	else if (rightobj->As<SpellItem>()->effects[0]->baseEffect->HasKeywordString("MagicDamageShock"))
		color = "<font color='#961696'>";
	DMG = color + DMG + "</font>";
	right = rightobj->GetName() + w + std::to_string(GetFormIndex(rightobj)) + w + DMG + w + zero + w + zero;
 }
 if (auto shouteq = GetShout(player)) {
	auto num = 1.f;
	int power = 0;
	SpellItem* spell = nullptr;
	if (shouteq->variations[2].word && IsWordUnlocked(0, 0, 0, shouteq->variations[2].word))
		spell = shouteq->variations[2].spell;
	else if (shouteq->variations[1].word && IsWordUnlocked(0, 0, 0, shouteq->variations[1].word))
		spell = shouteq->variations[1].spell;
	else if (shouteq->variations[0].word && IsWordUnlocked(0, 0, 0, shouteq->variations[0].word))
		spell = shouteq->variations[0].spell;
	if (spell && !spell->effects.empty() && spell->effects[0]) {
		BGSEntryPoint::HandleEntryPoint(BGSEntryPoint::ENTRY_POINT::kModSpellMagnitude, player, spell, nullptr, &num);
		power = (int)ceil((spell->effects[0]->effectItem.magnitude * num));
	}
		shout = shouteq->GetName() + w + zero + w + std::to_string(power);
	
 } else if (player->GetActorRuntimeData().selectedPower) {
	shout = player->GetActorRuntimeData().selectedPower->GetName() + w + std::to_string(1) + w + zero;
 } else {
	shout = "";
 }
 if (auto proj = player->GetCurrentAmmo())
	arrow = proj->GetName() + w + std::to_string(player->GetItemCount(proj));
 else
	arrow = "Null&0";
// int wind = 0;
// if (auto active_effects = player->GetActiveEffectList())
//	for (auto active_effect : *active_effects) {
//		if (active_effect->spell && active_effect->effect->baseEffect->HasKeywordString("aaMZkw_WindStoneStaks") &&
//			active_effect->spell->effects[0]->baseEffect == active_effect->effect->baseEffect)
//		wind += 1;
//	}
 //auto gv = TESForm::LookupByEditorID<TESGlobal>("aaMZgv_WidgetHealUse");
 //auto gv2 = TESForm::LookupByEditorID<TESGlobal>("aaMZgv_WidgetMagickaUse");
// auto gv3 = TESForm::LookupByEditorID<TESGlobal>("aaMZgv_WidgetStaminaUse");

 std::string hpnum = zero;
 std::string costnum = zero;
 std::string costnum2 = zero;
 if (LastUseMP && LastUseMP == LastUseST)
	LastUseMP = nullptr;
 if (LastUseST && LastUseST == LastUseHP)
	LastUseST = nullptr;
 if (LastUseMP && LastUseMP == LastUseHP)
	LastUseMP = nullptr;
 if (LastUseST && LastUseST == LastUseMP)
	LastUseST = nullptr;
 if (LastUseHP)
 hpnum = std::to_string(player->GetItemCount(LastUseHP));
 if (LastUseST)
 costnum2 = std::to_string(player->GetItemCount(LastUseST));
 if (LastUseMP)
 costnum = std::to_string(player->GetItemCount(LastUseMP));
 
 pot = hpnum + w + costnum2 + w + costnum;
 std::string result = arrow + "|" + shout + "|" + left + "|" + right + "|" + twohand + "|" + pot + "|" + zero + "|" + zero;

 const GFxValue weight{ result };
 widgetweight->uiMovie->Invoke("widget.Update", nullptr, &weight, 1);
 if (auto hudMenu = ui->GetMenu<RE::HUDMenu>(); hudMenu && hudMenu->uiMovie) {
 SetHudVisible(hudMenu->uiMovie.get(), "_root.HUDMovieBaseInstance.ArrowInfoInstance._visible",
	 !Settings::VisibleEquip_);
 }

}

void WidgetEquip::AdvanceMovie(const float interval, const uint32_t current_time)
{
    if (!STB::AdvanceWidget(*this, STB::Widget::Equip, frameState_, interval, current_time)) {
        return;
    }
    if (Settings::VisibleEquip_ && Settings::VisibleEquipKey && frameState_.elapsed >= 0.2f) {
        update();
        frameState_.elapsed = 0.0f;
    }
}

auto WidgetEquip::toggle_visibility(const bool mode) -> void
{
    STB::Hud().Visibility(mode);
}
