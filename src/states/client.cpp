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
#include <states/client.h>
#include <globals.h>
#include <framework.h>
#include <graphics.h>
#include <input.h>
#include <actions.h>
#include <objectmanager.h>
#include <stats.h>
#include <hud.h>
#include <buffer.h>
#include <assets.h>
#include <camera.h>
#include <program.h>
#include <menu.h>
#include <packet.h>
#include <ui/element.h>
#include <network/network.h>
#include <instances/map.h>
#include <instances/clientbattle.h>
#include <objects/player.h>
#include <objects/monster.h>
#include <states/null.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

_ClientState ClientState;

// Constructor
_ClientState::_ClientState()
:	CharacterSlot(0),
	IsTesting(false) {

}

// Initializes the state
void _ClientState::Init() {
	Menu.InitPlay();

	ClientTime = 0;
	Player = nullptr;

	ObjectManager = new _ObjectManager();
	Camera = new _Camera(glm::vec3(0, 0, CAMERA_DISTANCE), CAMERA_DIVISOR);
	Camera->CalculateFrustum(Graphics.AspectRatio);

	// Set up the HUD system
	HUD.Init();

	// Send character slot to play
	_Buffer Packet;
	Packet.Write<char>(Packet::CHARACTERS_PLAY);
	Packet.Write<char>(CharacterSlot);
	ClientNetwork->SendPacketToHost(&Packet);
}

// Shuts the state down
void _ClientState::Close() {

	ClientNetwork->Disconnect();
	HUD.Close();
	delete Camera;
	delete ObjectManager;
}

// Handles a connection to the server
void _ClientState::HandleConnect(ENetEvent *Event) {

}

// Handles a disconnection from the server
void _ClientState::HandleDisconnect(ENetEvent *Event) {

	Framework.ChangeState(&NullState);
}

// Handles a server packet
void _ClientState::HandlePacket(ENetEvent *Event) {
	//printf("HandlePacket: type=%d\n", Event->packet->data[0]);

	_Buffer Packet((char *)Event->packet->data, Event->packet->dataLength);
	int PacketType = Packet.Read<char>();
	switch(PacketType) {
		case Packet::WORLD_YOURCHARACTERINFO:
			HandleYourCharacterInfo(&Packet);
		break;
		case Packet::WORLD_CHANGEMAPS:
			HandleChangeMaps(&Packet);
		break;
		case Packet::WORLD_CREATEOBJECT:
			HandleCreateObject(&Packet);
		break;
		case Packet::WORLD_DELETEOBJECT:
			HandleDeleteObject(&Packet);
		break;
		case Packet::WORLD_OBJECTUPDATES:
			HandleObjectUpdates(&Packet);
		break;
		case Packet::WORLD_STARTBATTLE:
			HandleStartBattle(&Packet);
		break;
		case Packet::WORLD_HUD:
			HandleHUD(&Packet);
		break;
		case Packet::WORLD_POSITION:
			HandlePlayerPosition(&Packet);
		break;
		case Packet::BATTLE_TURNRESULTS:
			HandleBattleTurnResults(&Packet);
		break;
		case Packet::BATTLE_END:
			HandleBattleEnd(&Packet);
		break;
		case Packet::BATTLE_COMMAND:
			HandleBattleCommand(&Packet);
		break;
		case Packet::EVENT_START:
			HandleEventStart(&Packet);
		break;
		case Packet::INVENTORY_USE:
			HandleInventoryUse(&Packet);
		break;
		case Packet::CHAT_MESSAGE:
			HandleChatMessage(&Packet);
		break;
		case Packet::TRADE_REQUEST:
			HandleTradeRequest(&Packet);
		break;
		case Packet::TRADE_CANCEL:
			HandleTradeCancel(&Packet);
		break;
		case Packet::TRADE_ITEM:
			HandleTradeItem(&Packet);
		break;
		case Packet::TRADE_GOLD:
			HandleTradeGold(&Packet);
		break;
		case Packet::TRADE_ACCEPT:
			HandleTradeAccept(&Packet);
		break;
		case Packet::TRADE_EXCHANGE:
			HandleTradeExchange(&Packet);
		break;
	}
}

// Key events
void _ClientState::KeyEvent(const _KeyEvent &KeyEvent) {
	bool Handled = Graphics.Element->HandleKeyEvent(KeyEvent);

	if(!Handled) {
		if(Menu.GetState() != _Menu::STATE_NONE) {
			Menu.KeyEvent(KeyEvent);
			return;
		}
	}
	else {
		if(!HUD.IsChatting())
			HUD.ValidateTradeGold();
	}
}

// Mouse events
void _ClientState::MouseEvent(const _MouseEvent &MouseEvent) {
	FocusedElement = nullptr;
	Graphics.Element->HandleInput(MouseEvent.Pressed);

	// Pass to menu
	if(Menu.GetState() != _Menu::STATE_NONE) {
		Menu.MouseEvent(MouseEvent);
		return;
	}

	// Pass to hud
	HUD.MouseEvent(MouseEvent);
}

// Window events
void _ClientState::WindowEvent(uint8_t Event) {
	if(Camera && Event == SDL_WINDOWEVENT_SIZE_CHANGED)
		Camera->CalculateFrustum(Graphics.AspectRatio);
}

// Handle an input action
bool _ClientState::HandleAction(int InputType, int Action, int Value) {
	if(Value == 0)
		return true;

	// Handle enter key
	if(Action == _Actions::CHAT) {
		HUD.HandleEnter();
		return true;
	}

	// Grab all actions except escape
	if(HUD.IsChatting()) {
		if(Action == _Actions::MENU)
			HUD.CloseChat();

		return true;
	}

	// Pass to menu
	if(Menu.GetState() != _Menu::STATE_NONE) {
		Menu.HandleAction(InputType, Action, Value);
		return true;
	}

	switch(Player->State) {
		case _Player::STATE_WALK:
			switch(Action) {
				case _Actions::MENU:
					if(IsTesting)
						ClientNetwork->Disconnect();
					else
						HUD.ToggleMenu();
				break;
				case _Actions::INVENTORY:
					HUD.ToggleInventory();
				break;
				case _Actions::TELEPORT:
					HUD.ToggleTeleport();
				break;
				case _Actions::TRADE:
					HUD.ToggleTrade();
				break;
				case _Actions::SKILLS:
					HUD.ToggleSkills();
				break;
				case _Actions::ATTACK:
					SendAttackPlayer();
				break;
			}
		break;
		case _Player::STATE_BATTLE:
			((_ClientBattle *)Player->Battle)->HandleAction(Action);
		break;
		case _Player::STATE_TELEPORT:
			switch(Action) {
				case _Actions::UP:
				case _Actions::DOWN:
				case _Actions::LEFT:
				case _Actions::RIGHT:
				case _Actions::MENU:
				case _Actions::TELEPORT:
					HUD.ToggleTeleport();
				break;
			}
		break;
		case _Player::STATE_VENDOR:
			switch(Action) {
				case _Actions::UP:
				case _Actions::DOWN:
				case _Actions::LEFT:
				case _Actions::RIGHT:
				case _Actions::MENU:
				case _Actions::INVENTORY:
					HUD.CloseWindows();
				break;
			}
		break;
		case _Player::STATE_TRADER:
			switch(Action) {
				case _Actions::UP:
				case _Actions::DOWN:
				case _Actions::LEFT:
				case _Actions::RIGHT:
				case _Actions::MENU:
				case _Actions::INVENTORY:
					HUD.CloseWindows();
				break;
			}
		break;
		case _Player::STATE_TRADE:
			switch(Action) {
				case _Actions::UP:
				case _Actions::DOWN:
				case _Actions::LEFT:
				case _Actions::RIGHT:
				case _Actions::MENU:
				case _Actions::INVENTORY:
				case _Actions::TRADE:
					HUD.CloseWindows();
				break;
			}
		break;
		case _Player::STATE_BUSY:
			HUD.CloseWindows();
		break;
	}

	return true;
}

// Updates the current state
void _ClientState::Update(double FrameTime) {
	Graphics.Element->Update(FrameTime, Input.GetMouse());

	if(Camera && Player) {
		Camera->Set2DPosition(glm::vec2(Player->Position) + glm::vec2(+0.5f, +0.5f));
		Camera->Update(FrameTime);
	}

	// Handle input
	if(Menu.GetState() == _Menu::STATE_NONE) {
		switch(Player->State) {
			case _Player::STATE_WALK: {
				if(!HUD.IsChatting()) {
					if(Actions.GetState(_Actions::UP))
						SendMoveCommand(_Player::MOVE_UP);
					else if(Actions.GetState(_Actions::DOWN))
						SendMoveCommand(_Player::MOVE_DOWN);
					else if(Actions.GetState(_Actions::LEFT))
						SendMoveCommand(_Player::MOVE_LEFT);
					else if(Actions.GetState(_Actions::RIGHT))
						SendMoveCommand(_Player::MOVE_RIGHT);
				}
			} break;
			case _Player::STATE_BATTLE: {

				// Send key input
				if(!HUD.IsChatting()) {
					for(int i = 0; i < BATTLE_MAXSKILLS; i++) {
						if(Actions.GetState(_Actions::SKILL1+i)) {
							((_ClientBattle *)Player->Battle)->HandleAction(_Actions::SKILL1+i);
							break;
						}
					}
				}

				// Singleplayer check
				if(Player->Battle) {

					// Update the battle
					Player->Battle->Update(FrameTime);

					// Done with the battle
					if(Player->Battle->GetState() == _ClientBattle::STATE_DELETE) {
						delete Player->Battle;
						Player->Battle = nullptr;
						Player->State = _Player::STATE_WALK;
					}
				}
			} break;
		}
	}

	// Update and menu
	HUD.Update(FrameTime);
	Menu.Update(FrameTime);

	// Update objects
	ObjectManager->Update(FrameTime);

	ClientTime += FrameTime;
}

void _ClientState::Render(double BlendFactor) {
	if(!Player || !Player->Map || !Camera)
		return;

	Graphics.Setup3D();
	glm::vec3 LightPosition(glm::vec3(Player->Position, 1) + glm::vec3(0.5f, 0.5f, 0));
	glm::vec3 LightAttenuation(0.0f, 1.0f, 0.0f);

	Assets.Programs["pos_uv"]->LightAttenuation = LightAttenuation;
	Assets.Programs["pos_uv"]->LightPosition = LightPosition;
	Assets.Programs["pos_uv"]->AmbientLight = Player->Map->AmbientLight;
	Assets.Programs["pos_uv_norm"]->LightAttenuation = LightAttenuation;
	Assets.Programs["pos_uv_norm"]->LightPosition = LightPosition;
	Assets.Programs["pos_uv_norm"]->AmbientLight = Player->Map->AmbientLight;

	// Setup the viewing matrix
	Graphics.Setup3D();
	Camera->Set3DProjection(BlendFactor);
	Graphics.SetProgram(Assets.Programs["pos"]);
	glUniformMatrix4fv(Assets.Programs["pos"]->ViewProjectionTransformID, 1, GL_FALSE, glm::value_ptr(Camera->Transform));
	Graphics.SetProgram(Assets.Programs["pos_uv"]);
	glUniformMatrix4fv(Assets.Programs["pos_uv"]->ViewProjectionTransformID, 1, GL_FALSE, glm::value_ptr(Camera->Transform));
	Graphics.SetProgram(Assets.Programs["pos_uv_norm"]);
	glUniformMatrix4fv(Assets.Programs["pos_uv_norm"]->ViewProjectionTransformID, 1, GL_FALSE, glm::value_ptr(Camera->Transform));
	Graphics.SetProgram(Assets.Programs["text"]);
	glUniformMatrix4fv(Assets.Programs["text"]->ViewProjectionTransformID, 1, GL_FALSE, glm::value_ptr(Camera->Transform));

	// Draw map and objects
	Player->Map->Render(Camera);
	ObjectManager->Render(Player);

	Graphics.Setup2D();
	Graphics.SetProgram(Assets.Programs["text"]);
	glUniformMatrix4fv(Assets.Programs["text"]->ViewProjectionTransformID, 1, GL_FALSE, glm::value_ptr(Graphics.Ortho));

	// Draw HUD
	HUD.Render();

	// Draw states
	if(Player->Battle)
		((_ClientBattle *)Player->Battle)->Render();

	// Draw menu
	Menu.Render();
}

// Send the busy signal to server
void _ClientState::SendBusy(bool Value) {
	_Buffer Packet;
	Packet.Write<char>(Packet::WORLD_BUSY);
	Packet.Write<char>(Value);
	ClientNetwork->SendPacketToHost(&Packet);
}

// Called once to synchronize your stats with the servers
void _ClientState::HandleYourCharacterInfo(_Buffer *Packet) {

	// Get pack info
	int NetworkID = Packet->Read<char>();

	Player = new _Player();
	Player->Name = Packet->ReadString();
	Player->SetPortraitID(Packet->Read<int32_t>());
	Player->Experience = Packet->Read<int32_t>();
	Player->Gold = Packet->Read<int32_t>();
	Player->PlayTime = Packet->Read<int32_t>();
	Player->Deaths = Packet->Read<int32_t>();
	Player->MonsterKills = Packet->Read<int32_t>();
	Player->PlayerKills = Packet->Read<int32_t>();
	Player->Bounty = Packet->Read<int32_t>();
	HUD.SetPlayer(Player);

	// Read items
	int ItemCount = Packet->Read<char>();
	for(int i = 0; i < ItemCount; i++) {
		int Slot = Packet->Read<char>();
		int Count = (uint8_t)Packet->Read<char>();
		int ItemID = Packet->Read<int32_t>();
		Player->SetInventory(Slot, ItemID, Count);
	}

	// Read skills
	int SkillCount = Packet->Read<char>();
	for(int i = 0; i < SkillCount; i++) {
		int Points = Packet->Read<int32_t>();
		int Slot = Packet->Read<char>();
		Player->SetSkillLevel(Slot, Points);
	}

	// Read skill bar
	for(int i = 0; i < FIGHTER_MAXSKILLS; i++) {
		int SkillID = Packet->Read<char>();
		Player->SetSkillBar(i, Stats.GetSkill(SkillID));
	}

	Player->CalculateSkillPoints();
	Player->CalculatePlayerStats();
	Player->RestoreHealthMana();
	ObjectManager->AddObjectWithNetworkID(Player, NetworkID);
}

// Called when the player changes maps
void _ClientState::HandleChangeMaps(_Buffer *Packet) {
	if(!Player)
		return;

	// Load map
	int MapID = Packet->Read<int32_t>();

	// Delete old map and create new
	if(!Player->Map || Player->Map->ID != MapID) {

		if(Player->Map)
			delete Player->Map;

		Player->Map = new _Map(MapID);
	}

	// Clear out other objects
	ObjectManager->DeletesObjectsExcept(Player);

	// Get player count for map
	int PlayerCount = Packet->Read<int32_t>();

	// Spawn players
	int NetworkID;
	_Player *NewPlayer;
	glm::ivec2 GridPosition;
	for(int i = 0; i < PlayerCount; i++) {
		NetworkID = Packet->Read<char>();
		GridPosition.x = Packet->Read<char>();
		GridPosition.y = Packet->Read<char>();
		int Type = Packet->Read<char>();

		switch(Type) {
			case _Object::PLAYER: {
				std::string Name(Packet->ReadString());
				int PortraitID = Packet->Read<char>();
				int Invisible = Packet->ReadBit();

				// Information for your player
				if(NetworkID == Player->NetworkID) {
					Player->Position = GridPosition;
					Camera->ForcePosition(glm::vec3(GridPosition, CAMERA_DISTANCE) + glm::vec3(0.5, 0.5, 0));
				}
				else {

					NewPlayer = new _Player();
					NewPlayer->Position = GridPosition;
					NewPlayer->Name = Name;
					NewPlayer->Map = Player->Map;
					NewPlayer->SetPortraitID(PortraitID);
					NewPlayer->InvisPower = Invisible;
					ObjectManager->AddObjectWithNetworkID(NewPlayer, NetworkID);
				}
			}
			break;
		}
	}

	// Delete the battle
	if(Player->Battle) {
		delete Player->Battle;
		Player->Battle = nullptr;
	}

	Player->State = _Player::STATE_WALK;
}

// Creates an object
void _ClientState::HandleCreateObject(_Buffer *Packet) {

	// Read packet
	glm::ivec2 Position;
	int NetworkID = Packet->Read<char>();
	Position.x = Packet->Read<char>();
	Position.y = Packet->Read<char>();
	int Type = Packet->Read<char>();

	// Create the object
	_Object *NewObject = nullptr;
	switch(Type) {
		case _Object::PLAYER: {
			std::string Name(Packet->ReadString());
			int PortraitID = Packet->Read<char>();
			int Invisible = Packet->ReadBit();

			NewObject = new _Player();
			_Player *NewPlayer = (_Player *)NewObject;
			NewPlayer->Name = Name;
			NewPlayer->SetPortraitID(PortraitID);
			NewPlayer->InvisPower = Invisible;
			NewPlayer->Map = Player->Map;
		}
		break;
	}

	if(NewObject) {
		NewObject->Position = Position;

		// Add it to the manager
		ObjectManager->AddObjectWithNetworkID(NewObject, NetworkID);
	}

	//printf("HandleCreateObject: NetworkID=%d, Type=%d\n", NetworkID, Type); fflush(stdout);
}

// Deletes an object
void _ClientState::HandleDeleteObject(_Buffer *Packet) {
	int NetworkID = Packet->Read<char>();

	_Object *Object = ObjectManager->GetObjectFromNetworkID(NetworkID);
	if(Object) {
		if(Object->Type == _Object::PLAYER) {
			_Player *DeletedPlayer = (_Player *)Object;
			switch(Player->State) {
				case _Player::STATE_BATTLE:
					((_ClientBattle *)Player->Battle)->RemovePlayer(DeletedPlayer);
				break;
			}
		}
		Object->Deleted = true;
	}
	else
		printf("failed to delete object with networkid=%d\n", NetworkID);

	//printf("HandleDeleteObject: NetworkID=%d\n", NetworkID);
}

// Handles position updates from the server
void _ClientState::HandleObjectUpdates(_Buffer *Packet) {

	// Get object Count
	char ObjectCount = Packet->Read<char>();

	//printf("HandleObjectUpdates: ServerTime=%d, ClientTime=%d, ObjectCount=%d\n", ServerTime, ClientTime, ObjectCount);
	for(int i = 0; i < ObjectCount; i++) {
		glm::ivec2 Position;

		char NetworkID = Packet->Read<char>();
		int PlayerState = Packet->Read<char>();
		Position.x = Packet->Read<char>();
		Position.y = Packet->Read<char>();
		int Invisible = Packet->ReadBit();

		//printf("NetworkID=%d invis=%d\n", NetworkID, Invisible);

		_Player *OtherPlayer = (_Player *)ObjectManager->GetObjectFromNetworkID(NetworkID);
		if(OtherPlayer) {

			OtherPlayer->State = PlayerState;
			if(Player == OtherPlayer) {

				// Return from teleport state
				if(PlayerState == _Player::STATE_WALK && Player->State == _Player::STATE_TELEPORT) {
					Player->State = _Player::STATE_WALK;
				}
			}
			else {
				OtherPlayer->Position = Position;
				OtherPlayer->InvisPower = Invisible;
			}

			switch(PlayerState) {
				case _Player::STATE_WALK:
					OtherPlayer->StateImage = nullptr;
				break;
				case _Player::STATE_TRADE:
					OtherPlayer->StateImage = Assets.Textures["world/trade.png"];
				break;
				default:
					OtherPlayer->StateImage = Assets.Textures["world/busy.png"];
				break;
			}
		}
	}
}

// Handles the start of a battle
void _ClientState::HandleStartBattle(_Buffer *Packet) {

	// Already in a battle
	if(Player->Battle)
		return;

	// Create a new battle instance
	Player->Battle = new _ClientBattle();;

	// Get fighter count
	int FighterCount = Packet->Read<char>();

	// Get fighter information
	for(int i = 0; i < FighterCount; i++) {

		// Get fighter type
		int Type = Packet->ReadBit();
		int Side = Packet->ReadBit();
		if(Type == _Fighter::TYPE_PLAYER) {

			// Network ID
			int NetworkID = Packet->Read<char>();

			// Player stats
			int Health = Packet->Read<int32_t>();
			int MaxHealth = Packet->Read<int32_t>();
			int Mana = Packet->Read<int32_t>();
			int MaxMana = Packet->Read<int32_t>();

			// Get player object
			_Player *NewPlayer = (_Player *)ObjectManager->GetObjectFromNetworkID(NetworkID);
			if(NewPlayer != nullptr) {
				NewPlayer->Health = Health;
				NewPlayer->MaxHealth = MaxHealth;
				NewPlayer->Mana = Mana;
				NewPlayer->MaxMana = MaxMana;
				NewPlayer->Battle = Player->Battle;

				Player->Battle->AddFighter(NewPlayer, Side);
			}
		}
		else {

			// Monster ID
			int MonsterID = Packet->Read<int32_t>();
			_Monster *Monster = new _Monster(MonsterID);

			Player->Battle->AddFighter(Monster, Side);
		}
	}

	// Start the battle
	HUD.CloseWindows();
	((_ClientBattle *)Player->Battle)->StartBattle(Player);
}

// Handles the result of a turn in battle
void _ClientState::HandleBattleTurnResults(_Buffer *Packet) {

	// Check for a battle in progress
	if(!Player->Battle)
		return;

	((_ClientBattle *)Player->Battle)->ResolveTurn(Packet);
}

// Handles the end of a battle
void _ClientState::HandleBattleEnd(_Buffer *Packet) {

	// Check for a battle in progress
	if(!Player->Battle)
		return;

	((_ClientBattle *)Player->Battle)->EndBattle(Packet);
}

// Handles a battle command from other players
void _ClientState::HandleBattleCommand(_Buffer *Packet) {

	// Check for a battle in progress
	if(!Player->Battle)
		return;

	int Slot = Packet->Read<char>();
	int SkillID = Packet->Read<char>();
	((_ClientBattle *)Player->Battle)->HandleCommand(Slot, SkillID);
}

// Handles HUD updates
void _ClientState::HandleHUD(_Buffer *Packet) {
	Player->Experience = Packet->Read<int32_t>();
	Player->Gold = Packet->Read<int32_t>();
	Player->Health = Packet->Read<int32_t>();
	Player->Mana = Packet->Read<int32_t>();
	Player->HealthAccumulator = Packet->Read<float>();
	Player->ManaAccumulator = Packet->Read<float>();
	Player->CalculatePlayerStats();
}

// Handles player position
void _ClientState::HandlePlayerPosition(_Buffer *Packet) {
	glm::ivec2 GridPosition;
	GridPosition.x = Packet->Read<char>();
	GridPosition.y = Packet->Read<char>();
	Player->Position = GridPosition;
}

// Handles the start of an event
void _ClientState::HandleEventStart(_Buffer *Packet) {
	glm::ivec2 GridPosition;
	int Type = Packet->Read<char>();
	int Data = Packet->Read<int32_t>();
	GridPosition.x = Packet->Read<char>();
	GridPosition.y = Packet->Read<char>();
	Player->Position = GridPosition;

	switch(Type) {
		case _Map::EVENT_VENDOR:
			HUD.InitVendor(Data);
			Player->State = _Player::STATE_VENDOR;
		break;
		case _Map::EVENT_TRADER:
			HUD.InitTrader(Data);
			Player->State = _Player::STATE_TRADER;
		break;
	}
}

// Handles the use of an inventory item
void _ClientState::HandleInventoryUse(_Buffer *Packet) {
	int Slot = Packet->Read<char>();
	Player->UpdateInventory(Slot, -1);
}

// Handles a chat message
void _ClientState::HandleChatMessage(_Buffer *Packet) {

	// Read packet
	_ChatMessage Chat;
	Chat.Color = Packet->Read<glm::vec4>();
	Chat.Message = Packet->ReadString();
	Chat.Time = ClientTime;

	HUD.AddChatMessage(Chat);
}

// Handles a trade request
void _ClientState::HandleTradeRequest(_Buffer *Packet) {

	// Read packet
	int NetworkID = Packet->Read<char>();

	// Get trading player
	Player->TradePlayer = (_Player *)ObjectManager->GetObjectFromNetworkID(NetworkID);
	if(!Player->TradePlayer)
		return;

	// Get gold offer
	Player->TradePlayer->TradeGold = Packet->Read<int32_t>();
	for(int i = _Player::INVENTORY_TRADE; i < _Player::INVENTORY_COUNT; i++) {
		int ItemID = Packet->Read<int32_t>();
		int Count = 0;
		if(ItemID != 0)
			Count = Packet->Read<char>();

		Player->TradePlayer->SetInventory(i, ItemID, Count);
	}
}

// Handles a trade cancel
void _ClientState::HandleTradeCancel(_Buffer *Packet) {
	Player->TradePlayer = nullptr;

	// Reset agreement
	HUD.ResetAcceptButton();
}

// Handles a trade item update
void _ClientState::HandleTradeItem(_Buffer *Packet) {

	// Get trading player
	if(!Player->TradePlayer)
		return;

	// Get old slot information
	int OldItemID = Packet->Read<int32_t>();
	int OldSlot = Packet->Read<char>();
	int OldCount = 0;
	if(OldItemID > 0)
		OldCount = Packet->Read<char>();

	// Get new slot information
	int NewItemID = Packet->Read<int32_t>();
	int NewSlot = Packet->Read<char>();
	int NewCount = 0;
	if(NewItemID > 0)
		NewCount = Packet->Read<char>();

	// Update player
	Player->TradePlayer->SetInventory(OldSlot, OldItemID, OldCount);
	Player->TradePlayer->SetInventory(NewSlot, NewItemID, NewCount);

	// Reset agreement
	Player->TradePlayer->TradeAccepted = false;
	HUD.ResetAcceptButton();
}

// Handles a gold update from the trading player
void _ClientState::HandleTradeGold(_Buffer *Packet) {

	// Get trading player
	if(!Player->TradePlayer)
		return;

	// Set gold
	int Gold = Packet->Read<int32_t>();
	Player->TradePlayer->TradeGold = Gold;

	// Reset agreement
	Player->TradePlayer->TradeAccepted = false;
	HUD.ResetAcceptButton();
}

// Handles a trade accept
void _ClientState::HandleTradeAccept(_Buffer *Packet) {

	// Get trading player
	if(!Player->TradePlayer)
		return;

	// Set state
	bool Accepted = !!Packet->Read<char>();
	HUD.UpdateTradeStatus(Accepted);
}

// Handles a trade exchange
void _ClientState::HandleTradeExchange(_Buffer *Packet) {

	// Get gold offer
	int Gold = Packet->Read<int32_t>();
	Player->Gold = Gold;
	for(int i = _Player::INVENTORY_TRADE; i < _Player::INVENTORY_COUNT; i++) {
		int ItemID = Packet->Read<int32_t>();
		int Count = 0;
		if(ItemID != 0)
			Count = Packet->Read<char>();

		Player->SetInventory(i, ItemID, Count);
	}

	// Move traded items to backpack
	Player->MoveTradeToInventory();

	// Close window
	HUD.CloseTrade(false);
}

// Sends a move command to the server
void _ClientState::SendMoveCommand(int Direction) {

	if(Player->CanMove()) {

		// Move player locally
		if(Player->MovePlayer(Direction)) {
			_Buffer Packet;
			Packet.Write<char>(Packet::WORLD_MOVECOMMAND);
			Packet.Write<char>(Direction);
			ClientNetwork->SendPacketToHost(&Packet);
		}
	}
}

// Requests an attack to another a player
void _ClientState::SendAttackPlayer() {
	if(Player->CanAttackPlayer()) {
		Player->ResetAttackPlayerTime();
		_Buffer Packet;
		Packet.Write<char>(Packet::WORLD_ATTACKPLAYER);
		ClientNetwork->SendPacketToHost(&Packet);
	}
}
