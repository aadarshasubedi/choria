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
#include <objects/statuseffect.h>
#include <manager.h>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <string>
#include <map>

// Forward Declarations
class _Object;
class _Battle;
class _Buffer;
class _Font;

// Types of stats
enum class StatType : int {
	BUFF,
	BUFFLEVEL,
	BUFFDURATION,
	HEALTH,
	MAXHEALTH,
	MANA,
	MAXMANA,
	MINDAMAGE,
	MAXDAMAGE,
	MINDEFENSE,
	MAXDEFENSE,
	BATTLESPEED,
	HITCHANCE,
	EVASION,
	EXPERIENCE,
	GOLD,
	INVISIBLE,
	ACTIONBARSIZE,
	MISS,
	CRIT,
	COUNT,
};

enum class StatValueType : int {
	BOOLEAN,
	INTEGER,
	FLOAT,
	POINTER,
};

struct _StatStorage {
	StatType Type;
	StatValueType ValueType;
};

const std::map<std::string, StatType> StatStringToType = {
	{ "Buff",          StatType::BUFF          },
	{ "BuffLevel",     StatType::BUFFLEVEL     },
	{ "BuffDuration",  StatType::BUFFDURATION  },
	{ "Health",        StatType::HEALTH        },
	{ "MaxHealth",     StatType::MAXHEALTH     },
	{ "Mana",          StatType::MANA          },
	{ "MaxMana",       StatType::MAXMANA       },
	{ "MinDamage",     StatType::MINDAMAGE     },
	{ "MaxDamage",     StatType::MAXDAMAGE     },
	{ "MinDefense",    StatType::MINDEFENSE    },
	{ "MaxDefense",    StatType::MAXDEFENSE    },
	{ "BattleSpeed",   StatType::BATTLESPEED   },
	{ "HitChance",     StatType::HITCHANCE     },
	{ "Evasion",       StatType::EVASION       },
	{ "Experience",    StatType::EXPERIENCE    },
	{ "Gold",          StatType::GOLD          },
	{ "Invisible",     StatType::INVISIBLE     },
	{ "ActionBarSize", StatType::ACTIONBARSIZE },
	{ "Miss",          StatType::MISS          },
	{ "Crit",          StatType::CRIT          },
};

const _StatStorage StatValueTypes[] = {
	{ StatType::BUFF,          StatValueType::POINTER },
	{ StatType::BUFFLEVEL,     StatValueType::INTEGER },
	{ StatType::BUFFDURATION,  StatValueType::INTEGER },
	{ StatType::HEALTH,        StatValueType::FLOAT   },
	{ StatType::MAXHEALTH,     StatValueType::FLOAT   },
	{ StatType::MANA,          StatValueType::FLOAT   },
	{ StatType::MAXMANA,       StatValueType::FLOAT   },
	{ StatType::MINDAMAGE,     StatValueType::INTEGER },
	{ StatType::MAXDAMAGE,     StatValueType::INTEGER },
	{ StatType::MINDEFENSE,    StatValueType::INTEGER },
	{ StatType::MAXDEFENSE,    StatValueType::INTEGER },
	{ StatType::BATTLESPEED,   StatValueType::FLOAT   },
	{ StatType::HITCHANCE,     StatValueType::FLOAT   },
	{ StatType::EVASION,       StatValueType::FLOAT   },
	{ StatType::EXPERIENCE,    StatValueType::INTEGER },
	{ StatType::GOLD,          StatValueType::INTEGER },
	{ StatType::INVISIBLE,     StatValueType::BOOLEAN },
	{ StatType::ACTIONBARSIZE, StatValueType::INTEGER },
	{ StatType::MISS,          StatValueType::BOOLEAN },
	{ StatType::CRIT,          StatValueType::BOOLEAN },
};

union _Value {
	int Integer;
	float Float;
	void *Pointer;
};

// Stat changes
class _StatChange {

	public:

		_StatChange();

		void Reset();
		void Serialize(_Buffer &Data);
		void Unserialize(_Buffer &Data, _Manager<_Object> *Manager);

		int GetChangedFlag();
		bool HasStat(StatType Type);

		// Owner
		_Object *Object;

		// Data
		std::map<StatType, _Value> Values;
};

// Graphical stat change
class _StatChangeUI {

	public:

		_StatChangeUI();

		void Render(double BlendFactor);
		void SetText(const glm::vec4 &NegativeColor, const glm::vec4 &PositiveColor);

		_Object *Object;
		const _Font *Font;
		std::string Text;
		glm::vec4 Color;
		glm::vec2 StartPosition;
		glm::vec2 LastPosition;
		glm::vec2 Position;
		float Direction;
		double Time;
		double Timeout;
		int Change;

};

