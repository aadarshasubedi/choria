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
#include <log.h>
#include <memory>
#include <thread>
#include <list>

// Forward Declarations
class _ServerNetwork;
class _Buffer;
class _Peer;
class _Map;
class _Battle;
class _Stats;
class _Database;
class _Object;
struct _NetworkEvent;

// Server class
class _Server {

	public:

		_Server(uint16_t NetworkPort);
		~_Server();

		void Update(double FrameTime);
		void StartThread();
		void StopServer();

		// State
		bool Done;
		bool StartShutdown;
		uint16_t TimeSteps;
		double Time;
		_LogFile Log;

		// Stats
		_Database *Database;
		_Stats *Stats;

		// Network
		std::unique_ptr<_ServerNetwork> Network;

		// Map manager
		std::list<_Map *> Maps;
		std::list<_Battle *>Battles;
		uint8_t NextMapID;

		_Map *GetMap(int MapID);

	private:

		void SpawnPlayer(_Peer *Peer, int MapID, int EventType, int EventData);
		bool ValidatePeer(_Peer *Peer);
		void SendPlayerPosition(_Object *Player);

		void HandleConnect(_NetworkEvent &Event);
		void HandleDisconnect(_NetworkEvent &Event);
		void HandlePacket(_Buffer &Data, _Peer *Peer);

		void HandleLoginInfo(_Buffer &Data, _Peer *Peer);
		void HandleCharacterListRequest(_Buffer &Data, _Peer *Peer);
		void HandleCharacterPlay(_Buffer &Data, _Peer *Peer);
		void HandleCharacterCreate(_Buffer &Data, _Peer *Peer);
		void HandleCharacterDelete(_Buffer &Data, _Peer *Peer);
		void HandleMoveCommand(_Buffer &Data, _Peer *Peer);
		void HandleBattleCommand(_Buffer &Data, _Peer *Peer);
		void HandleBattleFinished(_Buffer &Data, _Peer *Peer);
		void HandleInventoryMove(_Buffer &Data, _Peer *Peer);
		void HandleInventoryUse(_Buffer &Data, _Peer *Peer);
		void HandleInventorySplit(_Buffer &Data, _Peer *Peer);
		void HandleEventEnd(_Buffer &Data, _Peer *Peer);
		void HandleVendorExchange(_Buffer &Data, _Peer *Peer);
		void HandleActionBar(_Buffer &Data, _Peer *Peer);
		void HandleSkillAdjust(_Buffer &Data, _Peer *Peer);
		void HandlePlayerBusy(_Buffer &Data, _Peer *Peer);
		void HandleAttackPlayer(_Buffer &Data, _Peer *Peer);
		void HandleChatMessage(_Buffer &Data, _Peer *Peer);
		void HandleTradeRequest(_Buffer &Data, _Peer *Peer);
		void HandleTradeCancel(_Buffer &Data, _Peer *Peer);
		void HandleTradeGold(_Buffer &Data, _Peer *Peer);
		void HandleTradeAccept(_Buffer &Data, _Peer *Peer);
		void HandleTeleport(_Buffer &Data, _Peer *Peer);
		void HandleTraderAccept(_Buffer &Data, _Peer *Peer);

		void SendCharacterList(_Peer *Peer);
		void SendObjectList(_Peer *Peer);
		void CreateDefaultDatabase();

		// Threading
		std::thread *Thread;
};