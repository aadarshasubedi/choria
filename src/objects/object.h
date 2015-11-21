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
#include <cstdint>
#include <glm/vec2.hpp>

// Forward Declarations
class _Map;

// Classes
class _Object {

	public:

		enum Type {
			NONE,
			FIGHTER,
			PLAYER,
			MONSTER,
		};

		virtual void RenderWorld(const _Object *TClientPlayer=nullptr) { }

		_Object(int TType);
		virtual ~_Object();

		virtual void Update(double FrameTime) { }

		int GetType() const { return Type; }

		void SetDeleted(bool Value) { Deleted = Value; }
		bool GetDeleted() const { return Deleted; }

		void SetNetworkID(char Value) { NetworkID = Value; }
		char GetNetworkID() const { return NetworkID; }

		void SetPosition(const glm::ivec2 &Position) { this->Position = Position; }
		const glm::ivec2 &GetPosition() const { return Position; }

		// Instances
		_Map *Map;

		// Properties
		int Type;

		// State
		bool Deleted;
		glm::ivec2 Position;

		// Networking
		char NetworkID;

	protected:


};
