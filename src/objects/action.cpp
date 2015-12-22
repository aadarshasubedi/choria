/******************************************************************************
* choria - https://github.com/jazztickets/choria
* Copyright (C) 2015  Alan Witkowski
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
#include <objects/action.h>
#include <objects/object.h>
#include <objects/inventory.h>
#include <objects/skill.h>
#include <objects/statuseffect.h>
#include <objects/buff.h>
#include <objects/item.h>
#include <constants.h>
#include <stats.h>
#include <buffer.h>

// Constructor
_ActionResult::_ActionResult() :
	LastPosition(0, 0),
	Position(0, 0),
	Texture(nullptr),
	SkillUsed(nullptr),
	ItemUsed(nullptr),
	Buff(nullptr),
	BuffLevel(0),
	BuffDuration(0),
	Time(0.0),
	Timeout(ACTIONRESULT_TIMEOUT),
	Speed(ACTIONRESULT_SPEED),
	Scope(ScopeType::ALL) {
}

// Return target type of action used
TargetType _ActionResult::GetUsedTargetType() {

	if(SkillUsed)
		return SkillUsed->TargetID;

	if(ItemUsed)
		return TargetType::ALLY;

	return TargetType::NONE;
}

// Serialize action
void _Action::Serialize(_Buffer &Data) {

	uint32_t SkillID = 0;
	if(Skill)
		SkillID = Skill->ID;

	uint32_t ItemID = 0;
	if(Item)
		ItemID = Item->ID;

	Data.Write<uint32_t>(SkillID);
	Data.Write<uint32_t>(ItemID);
}

// Unserialize action
void _Action::Unserialize(_Buffer &Data, _Stats *Stats) {

	uint32_t SkillID = Data.Read<uint32_t>();
	uint32_t ItemID = Data.Read<uint32_t>();

	Skill = Stats->Skills[SkillID];
	Item = Stats->Items[ItemID];
}

// Resolve action
bool _Action::Resolve(_Buffer &Data, _Object *Source, ScopeType Scope) {

	_ActionResult ActionResult;
	ActionResult.Source.Object = Source;
	ActionResult.Scope = Scope;
	ActionResult.SkillUsed = Source->Action.Skill;
	ActionResult.ItemUsed = Source->Action.Item;

	// Use item
	size_t Index;
	if(ActionResult.ItemUsed) {
		if(!ActionResult.ItemUsed->CanUse(Source->Scripting, ActionResult) || !ActionResult.Source.Object->Inventory->FindItem(ActionResult.ItemUsed, Index))
			return false;

		ActionResult.Source.Object->Inventory->DecrementItemCount(Index, -1);
	}

	// Apply costs
	if(ActionResult.SkillUsed) {
		if(!ActionResult.SkillUsed->CanUse(Source->Scripting, ActionResult))
			return false;

		ActionResult.SkillUsed->ApplyCost(Source->Scripting, ActionResult);
	}

	ActionResult.Source.Object->UpdateHealth(ActionResult.Source.Health);
	ActionResult.Source.Object->UpdateMana(ActionResult.Source.Mana);

	// Build packet for results
	Data.Write<PacketType>(PacketType::ACTION_RESULTS);

	// Write action used
	uint32_t SkillID = ActionResult.SkillUsed ? ActionResult.SkillUsed->ID : 0;
	uint32_t ItemID = ActionResult.ItemUsed ? ActionResult.ItemUsed->ID : 0;
	Data.Write<uint32_t>(SkillID);
	Data.Write<uint32_t>(ItemID);

	// Write source updates
	ActionResult.Source.Serialize(Data);
	Data.Write<int32_t>(ActionResult.Source.Object->Health);
	Data.Write<int32_t>(ActionResult.Source.Object->Mana);

	// Update each target
	Data.Write<uint8_t>((uint8_t)Source->Targets.size());
	for(auto &Target : Source->Targets) {
		ActionResult.Target.Object = Target;

		// Update objects
		if(ActionResult.SkillUsed) {
			ActionResult.SkillUsed->Use(Source->Scripting, ActionResult);
		}
		else if(ActionResult.ItemUsed) {
			ActionResult.ItemUsed->Use(Source->Scripting, ActionResult);
		}

		// Update target
		ActionResult.Target.Object->UpdateHealth(ActionResult.Target.Health);
		ActionResult.Target.Object->UpdateMana(ActionResult.Target.Mana);

		ActionResult.Target.Serialize(Data);
		Data.Write<int32_t>(ActionResult.Target.Object->Health);
		Data.Write<int32_t>(ActionResult.Target.Object->Mana);

		// Add buffs
		if(ActionResult.Buff) {
			_StatusEffect *StatusEffect = new _StatusEffect();
			StatusEffect->Buff = ActionResult.Buff;
			StatusEffect->Level = ActionResult.BuffLevel;
			StatusEffect->Count = ActionResult.BuffDuration;
			bool Added = ActionResult.Target.Object->AddStatusEffect(StatusEffect);

			Data.Write<uint32_t>(StatusEffect->Buff->ID);
			Data.Write<int>(StatusEffect->Level);
			Data.Write<int>(StatusEffect->Count);

			if(!Added)
				delete StatusEffect;
		}
		else
			Data.Write<uint32_t>(0);
	}

	// Reset object
	Source->TurnTimer = 0.0;
	Source->Action.Unset();
	Source->Targets.clear();

	return true;
}
