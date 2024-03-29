/******************************************************************************
* choria - https://github.com/jazztickets/choria
* Copyright (C) 2016  Alan Witkowski
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#include <objects/item.h>
#include <objects/object.h>
#include <objects/inventory.h>
#include <ui/label.h>
#include <ui/element.h>
#include <scripting.h>
#include <stats.h>
#include <constants.h>
#include <font.h>
#include <stats.h>
#include <hud.h>
#include <utils.h>
#include <scripting.h>
#include <graphics.h>
#include <input.h>
#include <assets.h>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <iostream>

// Draw tooltip
void _Item::DrawTooltip(_Scripting *Scripting, const _Object *Player, const _Cursor &Tooltip) const {
	if(!Player)
		return;

	_Element *TooltipElement = Assets.Elements["element_item_tooltip"];
	_Label *TooltipName = Assets.Labels["label_item_tooltip_name"];
	_Label *TooltipType = Assets.Labels["label_item_tooltip_type"];
	TooltipElement->SetVisible(true);

	// Get window dimensions
	glm::vec2 Size = TooltipElement->Size;

	// Set label values
	TooltipName->Text = Name;
	TooltipType->Text = "";
	if(Type != ItemType::NONE)
		TooltipType->Text = Player->Stats->ItemTypes[(uint32_t)Type];

	// Set window width
	_TextBounds TextBounds;
	Assets.Fonts["hud_medium"]->GetStringDimensions(TooltipName->Text, TextBounds);
	Size.x = 250;
	float SidePadding = 15;
	float SpacingY = 25;
	Size.x = std::max(Size.x, (float)TextBounds.Width) + SidePadding * 2;
	if(ResistanceTypeID)
		Size.x += 25;

	// Position window
	glm::vec2 WindowOffset = Input.GetMouse();
	WindowOffset.x += INVENTORY_TOOLTIP_OFFSET;
	WindowOffset.y += -(TooltipElement->Bounds.End.y - TooltipElement->Bounds.Start.y) / 2;

	// Reposition window if out of bounds
	if(WindowOffset.x + Size.x > Graphics.Element->Bounds.End.x - INVENTORY_TOOLTIP_PADDING)
		WindowOffset.x -= Size.x + INVENTORY_TOOLTIP_OFFSET + INVENTORY_TOOLTIP_PADDING;
	if(WindowOffset.y + Size.y > Graphics.Element->Bounds.End.y - INVENTORY_TOOLTIP_PADDING)
		WindowOffset.y -= Size.y + INVENTORY_TOOLTIP_OFFSET - (TooltipElement->Bounds.End.y - TooltipElement->Bounds.Start.y) / 2;

	TooltipElement->SetOffset(WindowOffset);
	TooltipElement->SetWidth(Size.x);

	// Render tooltip
	TooltipElement->Render();
	TooltipElement->SetVisible(false);

	// Set draw position to center of window
	glm::vec2 DrawPosition(TooltipElement->Size.x / 2 + WindowOffset.x, TooltipType->Bounds.End.y);
	DrawPosition.y += 40;

	// Draw target text
	if(TargetID != TargetType::NONE) {
		DrawPosition.y -= 20;
		std::string InfoText = "Target " + Player->Stats->TargetTypes[(uint32_t)TargetID];
		Assets.Fonts["hud_small"]->DrawText(InfoText, DrawPosition, COLOR_WHITE, CENTER_BASELINE);
		DrawPosition.y += 40;
	}

	// Get level of item or skill
	int32_t DrawLevel = Level;
	bool IsLocked = false;
	bool ShowLevel = false;
	if(IsSkill()) {

		// Get skill level
		auto SkillIterator = Player->Skills.find(ID);
		if(SkillIterator != Player->Skills.end())
			DrawLevel = SkillIterator->second;
		else
			IsLocked = true;

		// For skills set minimum of level 1
		DrawLevel = std::max(DrawLevel, 1);

		// Show vendor skills at level 1
		if(Tooltip.Window == _HUD::WINDOW_INVENTORY|| Tooltip.Window == _HUD::WINDOW_VENDOR)
			DrawLevel = 1;

		// Determine whether to show level
		if(Tooltip.Window == _HUD::WINDOW_SKILLS || Tooltip.Window == _HUD::WINDOW_ACTIONBAR)
			ShowLevel = true;
	}

	// Draw description
	DrawDescription(Scripting, DrawPosition, DrawLevel, ShowLevel, Size.x - SidePadding * 2, SpacingY);

	// Draw next level description
	if(IsSkill() && Tooltip.Window == _HUD::WINDOW_SKILLS) {
		DrawDescription(Scripting, DrawPosition, DrawLevel+1, true, Size.x - SidePadding * 2, SpacingY);
	}

	glm::vec2 Spacing(10, 0);

	// Render damage
	bool StatDrawn = false;
	if(MinDamage != 0 || MaxDamage != 0) {
		std::stringstream Buffer;
		if(MinDamage != MaxDamage)
			Buffer << MinDamage << " - " << MaxDamage;
		else
			Buffer << MinDamage;

		Assets.Fonts["hud_medium"]->DrawText("Damage", DrawPosition + -Spacing, glm::vec4(1.0f), RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, glm::vec4(1.0f), LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}

	// Render defense
	if(MinDefense != 0 || MaxDefense != 0) {
		std::stringstream Buffer;
		if(MinDefense != MaxDefense)
			Buffer << MinDefense << " - " << MaxDefense;
		else
			Buffer << MinDefense;

		Assets.Fonts["hud_medium"]->DrawText("Defense", DrawPosition + -Spacing, glm::vec4(1.0f), RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, glm::vec4(1.0f), LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}

	// Boosts
	if(MaxHealth > 0) {
		std::stringstream Buffer;
		Buffer << "+" << MaxHealth;
		Assets.Fonts["hud_medium"]->DrawText("Health", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}
	if(MaxMana > 0) {
		std::stringstream Buffer;
		Buffer << "+" << MaxMana;
		Assets.Fonts["hud_medium"]->DrawText("Mana", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}
	if(ResistanceTypeID) {
		std::stringstream Buffer;
		Buffer << (Resistance < 0 ? "" : "+") << std::setprecision(2) << Resistance * 100 << "%";
		Assets.Fonts["hud_medium"]->DrawText(Player->Stats->DamageTypes[ResistanceTypeID] + " Resist", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}
	if(MoveSpeed != 0.0f) {
		std::stringstream Buffer;
		Buffer << (BattleSpeed < 0 ? "" : "+") << std::setprecision(2) << MoveSpeed * 100 << "%";
		Assets.Fonts["hud_medium"]->DrawText("Move Speed", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}
	if(BattleSpeed != 0.0) {
		std::stringstream Buffer;
		Buffer << (BattleSpeed < 0 ? "" : "+") << std::setprecision(2) << BattleSpeed * 100 << "%";
		Assets.Fonts["hud_medium"]->DrawText("Battle Speed", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}
	if(HealthRegen != 0.0f) {
		std::stringstream Buffer;
		Buffer << "+" << Round(HealthRegen);
		Assets.Fonts["hud_medium"]->DrawText("Health Regen", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}
	if(ManaRegen != 0.0f) {
		std::stringstream Buffer;
		Buffer << "+" << Round(ManaRegen);
		Assets.Fonts["hud_medium"]->DrawText("Mana Regen", DrawPosition + -Spacing, COLOR_WHITE, RIGHT_BASELINE);
		Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition + Spacing, COLOR_WHITE, LEFT_BASELINE);
		DrawPosition.y += SpacingY;
		StatDrawn = true;
	}

	if(StatDrawn)
		DrawPosition.y += SpacingY;

	// Vendors
	if(Player->Vendor) {
		std::stringstream Buffer;
		if(Tooltip.Window == _HUD::WINDOW_VENDOR) {
			Buffer << "Buy " << Tooltip.Count << "x for " << Tooltip.Cost << " gold";
			Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition, COLOR_GOLD, CENTER_BASELINE);
			DrawPosition.y += SpacingY;
			Assets.Fonts["hud_small"]->DrawText("Right-click to buy", DrawPosition, COLOR_GRAY, CENTER_BASELINE);
			DrawPosition.y += SpacingY;
		}
		else if(Tooltip.Window == _HUD::WINDOW_INVENTORY) {
			Buffer << "Sell for " << Tooltip.Cost << " gold";
			Assets.Fonts["hud_medium"]->DrawText(Buffer.str().c_str(), DrawPosition, COLOR_GOLD, CENTER_BASELINE);
			DrawPosition.y += SpacingY;
			Assets.Fonts["hud_small"]->DrawText("Shift+Right-click to sell", DrawPosition, COLOR_GRAY, CENTER_BASELINE);
			DrawPosition.y += SpacingY;
		}
	}

	// Draw help text
	std::string InfoText;
	switch(Type) {
		case ItemType::ONEHANDED_WEAPON:
		case ItemType::TWOHANDED_WEAPON:
		case ItemType::HELMET:
		case ItemType::ARMOR:
		case ItemType::BOOTS:
		case ItemType::SHIELD:
			if(Tooltip.Window == _HUD::WINDOW_INVENTORY && Tooltip.Slot >= InventoryType::BAG)
				InfoText = "Right-click to equip";
		break;
		case ItemType::CONSUMABLE:
			if(Tooltip.Window == _HUD::WINDOW_INVENTORY && CheckScope(ScopeType::WORLD))
				InfoText = "Right-click to use";
			else if(Tooltip.Window == _HUD::WINDOW_ACTIONBAR && CheckScope(ScopeType::WORLD))
				InfoText = "Left-click to use";
		break;
		case ItemType::SKILL:
			if(Tooltip.Window == _HUD::WINDOW_ACTIONBAR) {
				if(CheckScope(ScopeType::WORLD) && TargetID != TargetType::NONE)
					InfoText = "Left-click to use";
			}
			else if(Tooltip.Window == _HUD::WINDOW_SKILLS) {
				if(TargetID == TargetType::NONE)
					InfoText = "Passive skills must be equipped";
			}
			else {
				if(IsLocked) {
					if(Tooltip.Window == _HUD::WINDOW_INVENTORY)
						InfoText = "Right-click to learn";
				}
				else
					InfoText = "Already learned";
			}
		break;
		case ItemType::UNLOCKABLE: {
			if(!Player->HasUnlocked(this))
				InfoText = "Right-click to unlock";
			else
				InfoText = "Already unlocked";
		} break;
		default:
		break;
	}

	if(InfoText.length()) {
		Assets.Fonts["hud_small"]->DrawText(InfoText, DrawPosition, COLOR_GRAY, CENTER_BASELINE);
		DrawPosition.y += 20;
	}

	if(Tooltip.Window == _HUD::WINDOW_INVENTORY && Tooltip.Count > 1) {
		Assets.Fonts["hud_small"]->DrawText("Ctrl+click to split", DrawPosition, COLOR_GRAY, CENTER_BASELINE);
		DrawPosition.y += SpacingY;
	}

	if(!Tradable && (Tooltip.Window == _HUD::WINDOW_INVENTORY || Tooltip.Window == _HUD::WINDOW_VENDOR || Tooltip.Window == _HUD::WINDOW_TRADER)) {
		Assets.Fonts["hud_small"]->DrawText("Untradable", DrawPosition, COLOR_RED, CENTER_BASELINE);
		DrawPosition.y += 20;
	}
}

// Draw item description
void _Item::DrawDescription(_Scripting *Scripting, glm::vec2 &DrawPosition, int DrawLevel, bool ShowLevel, float Width, float SpacingY) const {

	// Check for scripting function
	std::string Info = "";
	if(Scripting->StartMethodCall(Script, "GetInfo")) {

		// Get description from script
		Scripting->PushInt(DrawLevel);
		Scripting->MethodCall(1, 1);
		Info = Scripting->GetString(1);
		Scripting->FinishMethodCall();

		// Draw level text
		if(ShowLevel) {
			Assets.Fonts["hud_small"]->DrawText("Level " + std::to_string(DrawLevel), DrawPosition, COLOR_GRAY, CENTER_BASELINE);
			DrawPosition.y += SpacingY;
		}

		std::stringstream Buffer(Info);
		std::string Token;

		// Draw description
		float TextSpacingY = 18;
		while(std::getline(Buffer, Token, '\n')) {
			std::list<std::string> Strings;
			Assets.Fonts["hud_small"]->BreakupString(Token, Width, Strings, true);
			for(const auto &LineToken : Strings) {
				Assets.Fonts["hud_small"]->DrawTextFormatted(LineToken, DrawPosition, CENTER_BASELINE);
				DrawPosition.y += TextSpacingY;
			}
		}

		DrawPosition.y += SpacingY;
	}
}

// Return a valid equipment slot for an item
size_t _Item::GetEquipmentSlot() const {

	size_t Slot = (size_t)-1;
	switch(Type) {
		case ItemType::HELMET:
			Slot = InventoryType::HEAD;
		break;
		case ItemType::ARMOR:
			Slot = InventoryType::BODY;
		break;
		case ItemType::BOOTS:
			Slot = InventoryType::LEGS;
		break;
		case ItemType::ONEHANDED_WEAPON:
		case ItemType::TWOHANDED_WEAPON:
			Slot = InventoryType::HAND1;
		break;
		case ItemType::SHIELD:
			Slot = InventoryType::HAND2;
		break;
		case ItemType::RING:
			Slot = InventoryType::RING1;
		break;
		default:
		break;
	}

	return Slot;
}

// Returns the item's price to/from a vendor
int _Item::GetPrice(const _Vendor *Vendor, int QueryCount, bool Buy) const {
	if(!Vendor)
		return 0;

	// Calculate
	float Percent;
	if(Buy)
		Percent = Vendor->BuyPercent;
	else
		Percent = Vendor->SellPercent;

	int Price = (int)(Cost * Percent) * QueryCount;

	// Cap
	if(Price < 0)
		Price = 0;
	else if(Price > PLAYER_MAX_GOLD)
		Price = PLAYER_MAX_GOLD;

	return Price;
}

// Return true if the item can be used
bool _Item::CanUse(_Scripting *Scripting, _ActionResult &ActionResult) const {
	_Object *Object = ActionResult.Source.Object;
	if(!Object)
		return false;

	// Unlocking skill for the first time
	if(IsSkill() && ActionResult.ActionUsed.InventorySlot != -1) {
		return !Object->HasLearned(this);
	}

	// Unlocking item
	if(IsUnlockable()) {
		return !Object->HasUnlocked(this);
	}

	// Check scope
	if(!CheckScope(ActionResult.Scope))
		return false;

	// Check script's function
	if(Scripting->StartMethodCall(Script, "CanUse")) {
		Scripting->PushInt(ActionResult.ActionUsed.Level);
		Scripting->PushObject(ActionResult.Source.Object);
		Scripting->MethodCall(2, 1);
		int Value = Scripting->GetBoolean(1);
		Scripting->FinishMethodCall();

		return Value;
	}

	return true;
}

// Check if the item can be used in the given scope
bool _Item::CheckScope(ScopeType CheckScope) const {

	// Check scope
	if(Scope == ScopeType::NONE || (Scope != ScopeType::ALL && Scope != CheckScope))
		return false;

	return true;
}

// Apply the cost
void _Item::ApplyCost(_Scripting *Scripting, _ActionResult &ActionResult) const {
	if(Scripting->StartMethodCall(Script, "ApplyCost")) {
		Scripting->PushInt(ActionResult.ActionUsed.Level);
		Scripting->PushActionResult(&ActionResult);
		Scripting->MethodCall(2, 1);
		Scripting->GetActionResult(1, ActionResult);
		Scripting->FinishMethodCall();
	}
}

// Use an item
void _Item::Use(_Scripting *Scripting, _ActionResult &ActionResult) const {
	if(Scripting->StartMethodCall(Script, "Use")) {
		Scripting->PushInt(ActionResult.ActionUsed.Level);
		Scripting->PushObject(ActionResult.Source.Object);
		Scripting->PushObject(ActionResult.Target.Object);
		Scripting->PushActionResult(&ActionResult);
		Scripting->MethodCall(4, 1);
		Scripting->GetActionResult(1, ActionResult);
		Scripting->FinishMethodCall();
	}
}

// Get passive stats
void _Item::Stats(_Scripting *Scripting, _ActionResult &ActionResult) const {
	if(Scripting->StartMethodCall(Script, "Stats")) {
		Scripting->PushInt(ActionResult.ActionUsed.Level);
		Scripting->PushObject(ActionResult.Source.Object);
		Scripting->PushStatChange(&ActionResult.Source);
		Scripting->MethodCall(3, 1);
		Scripting->GetStatChange(1, ActionResult.Source);
		Scripting->FinishMethodCall();
	}
}
