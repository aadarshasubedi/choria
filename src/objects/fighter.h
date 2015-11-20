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

// Constants
const int FIGHTER_MAXSKILLS = 8;

// Forward Declarations
class _Battle;
class _Skill;
class _Texture;
struct _FighterResult;

// Classes
class _Fighter {

	public:

		enum FighterType {
			TYPE_PLAYER,
			TYPE_MONSTER,
		};

		_Fighter(int Type);
		virtual ~_Fighter();

		// Render
		void RenderBattle(bool ShowResults, float TimerPercent, _FighterResult *Result, bool TTarget);
		void SetOffset(const glm::ivec2 &Position) { Offset = Position; }

		// Stats
		void UpdateHealth(int Value);
		void UpdateMana(int Value);
		void RestoreHealthMana();

		void UpdateRegen(int &THealthUpdate, int &TManaUpdate);

		int GetLevel() const { return Level; }

		// Battles
		_Battle *GetBattle();
		void SetCommand(int TCommand) { Command = TCommand; }
		virtual int GetCommand() { return Command; }
		void SetSkillUsed(const _Skill *TSkill) { SkillUsed = TSkill; }
		const _Skill *GetSkillUsed() const { return SkillUsed; }
		void SetSkillUsing(const _Skill *TSkill) { SkillUsing = TSkill; }
		const _Skill *GetSkillUsing() const { return SkillUsing; }
		int GetSide() const { return BattleSlot & 1; }
		int GenerateDamage();
		int GenerateDefense();

		virtual int GetExperienceGiven() const { return 0; }
		virtual int GetGoldGiven() const { return 0; }

		virtual void UpdateExperience(int Value) { }
		virtual int GetSkillLevel(int Slot) const { return 1; }

		// Skills
		void SetSkillBar(int Slot, const _Skill *Skill) { SkillBar[Slot] = Skill; }
		const _Skill *GetSkillBar(int Slot);
		int GetSkillBarID(int Slot);

		// Objects
		int Type;

		// Action bar
		const _Skill *SkillBar[FIGHTER_MAXSKILLS];

		// Stats
		std::string Name;
		int Level;
		int Health, MaxHealth;
		int Mana, MaxMana;
		int MinDamage, MaxDamage;
		int MinDefense, MaxDefense;
		float HealthRegen, ManaRegen, HealthAccumulator, ManaAccumulator;

		// Battle
		_Battle *Battle;
		int Command, Target, BattleSlot;
		const _Skill *SkillUsing, *SkillUsed;

		// Render
		const _Texture *Portrait;
		glm::ivec2 Offset;

	protected:

};
