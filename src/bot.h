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
#pragma once

// Libraries
#include <packet.h>
#include <manager.h>
#include <string>
#include <memory>
#include <list>
#include <glm/vec2.hpp>

// Forward Declarations
class _Stats;
class _ClientNetwork;
class _Buffer;
class _Object;
class _Map;
class _Stats;
class _Scripting;

namespace micropather {
	class MicroPather;
}

enum class BotStateType : int {
	IDLE,
	MOVING
};

// Bot class
class _Bot {

	public:

		_Bot(_Stats *Stats, const std::string &HostAddress, uint16_t Port);
		~_Bot();

		// Update
		void Update(double FrameTime);

		// Network
		void HandlePacket(_Buffer &Data);
		void AssignPlayer(_Object *Object);
		_Object *CreateObject(_Buffer &Data, NetworkIDType NetworkID);

		// AI
		void MoveTo(const glm::ivec2 &StartPosition, const glm::ivec2 &EndPosition);
		int GetNextInputState();

		std::unique_ptr<_ClientNetwork> Network;

		_Manager<_Object> *ObjectManager;

		_Scripting *Scripting;
		_Map *Map;
		_Object *Player;
		_Stats *Stats;

		micropather::MicroPather *Pather;
		std::list<void *> Path;
		BotStateType BotState;

		std::string Username;
		std::string Password;
};
