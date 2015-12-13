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
#include <objects/skill.h>
#include <objects/item.h>
#include <stats.h>
#include <buffer.h>

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