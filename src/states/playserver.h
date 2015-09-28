/******************************************************************************
*	choria - https://github.com/jazztickets/choria
*	Copyright (C) 2015  Alan Witkowski
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#pragma once

// Libraries
#include <state.h>
#include <thread>

// Forward Declarations
class DatabaseClass;
class ObjectManagerClass;
class InstanceClass;
class _Packet;
class ObjectClass;
class PlayerClass;
class ServerBattleClass;

// Classes
class _PlayServerState : public _State {

	public:

		int Init();
		int Close();

		void HandleConnect(ENetEvent *TEvent);
		void HandleDisconnect(ENetEvent *TEvent);
		void HandlePacket(ENetEvent *TEvent);

		void Update(uint32_t TDeltaTime);
		void DeleteObject(ObjectClass *TObject);

		void PlayerTownPortal(PlayerClass *TPlayer);
		uint32_t GetServerTime() const { return ServerTime; }

		void StartCommandThread();
		void StopServer() { StopRequested = true; }

	private:

		void CreateDefaultDatabase();

		void HandleLoginInfo(_Packet *TPacket, ENetPeer *TPeer);
		void HandleCharacterListRequest(_Packet *TPacket, ENetPeer *TPeer);
		void HandleCharacterSelect(_Packet *TPacket, ENetPeer *TPeer);
		void HandleCharacterDelete(_Packet *TPacket, ENetPeer *TPeer);
		void HandleCharacterCreate(_Packet *TPacket, ENetPeer *TPeer);
		void HandleMoveCommand(_Packet *TPacket, ENetPeer *TPeer);
		void HandleBattleCommand(_Packet *TPacket, ENetPeer *TPeer);
		void HandleBattleFinished(_Packet *TPacket, ENetPeer *TPeer);
		void HandleInventoryMove(_Packet *TPacket, ENetPeer *TPeer);
		void HandleInventoryUse(_Packet *TPacket, ENetPeer *TPeer);
		void HandleInventorySplit(_Packet *TPacket, ENetPeer *TPeer);
		void HandleEventEnd(_Packet *TPacket, ENetPeer *TPeer);
		void HandleVendorExchange(_Packet *TPacket, ENetPeer *TPeer);
		void HandleSkillBar(_Packet *TPacket, ENetPeer *TPeer);
		void HandleSkillAdjust(_Packet *TPacket, ENetPeer *TPeer);
		void HandlePlayerBusy(_Packet *TPacket, ENetPeer *TPeer);
		void HandleAttackPlayer(_Packet *TPacket, ENetPeer *TPeer);
		void HandleChatMessage(_Packet *TPacket, ENetPeer *TPeer);
		void HandleTradeRequest(_Packet *TPacket, ENetPeer *TPeer);
		void HandleTradeCancel(_Packet *TPacket, ENetPeer *TPeer);
		void HandleTradeGold(_Packet *TPacket, ENetPeer *TPeer);
		void HandleTradeAccept(_Packet *TPacket, ENetPeer *TPeer);
		void HandleTownPortal(_Packet *TPacket, ENetPeer *TPeer);
		void HandleTraderAccept(_Packet *TPacket, ENetPeer *TPeer);

		void SendPlayerPosition(PlayerClass *TPlayer);
		void SpawnPlayer(PlayerClass *TPlayer, int TNewMapID, int TEventType, int TEventData);
		void SendHUD(PlayerClass *TPlayer);
		void SendCharacterList(PlayerClass *TPlayer);
		void SendEvent(PlayerClass *TPlayer, int TType, int TData);
		void SendTradeInformation(PlayerClass *TSender, PlayerClass *TReceiver);

		void BuildTradeItemsPacket(PlayerClass *TPlayer, _Packet *TPacket, int TGold);

		void RemovePlayerFromBattle(PlayerClass *TPlayer);

		DatabaseClass *Database;
		ObjectManagerClass *ObjectManager;
		InstanceClass *Instances;

		bool StopRequested;

		std::thread *CommandThread;

		uint32_t ServerTime;
};

extern _PlayServerState PlayServerState;
