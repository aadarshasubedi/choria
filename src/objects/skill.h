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
#pragma once

// Libraries
#include <texture.h>

// Forward Declarations
class _Fighter;
struct _Vendor;
struct _FighterResult;

// Classes
class _Skill {
	friend class _Stats;

	public:

		enum SkillType {
			TYPE_ATTACK,
			TYPE_SPELL,
			TYPE_PASSIVE,
			TYPE_USEPOTION,
		};

		int GetManaCost(int TLevel) const;
		int GetPower(int TLevel) const;
		void GetPowerRange(int TLevel, int &Min, int &Max) const;
		void GetPowerRangeRound(int TLevel, int &Min, int &Max) const;
		void GetPowerRange(int TLevel, float &Min, float &Max) const;

		void ResolveSkill(_FighterResult *Result, _FighterResult *TTargetResult) const;
		bool CanUse(_Fighter *TFighter) const;

		int ID;
		int Type;
		std::string Name;
		std::string Info;
		const _Texture *Image;
		int SkillCost;

	private:

		float ManaCostBase;
		float ManaCost;
		float PowerBase;
		float PowerRangeBase;
		float Power;
		float PowerRange;
};
