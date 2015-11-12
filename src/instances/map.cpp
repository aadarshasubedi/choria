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
#include <instances/map.h>
#include <globals.h>
#include <graphics.h>
#include <constants.h>
#include <stats.h>
#include <network/network.h>
#include <buffer.h>
#include <states/playserver.h>
#include <objects/player.h>
#include <texture.h>
#include <fstream>
#include <limits>

using namespace irr;

// Constructor for the map editor: new map
_Map::_Map(const core::stringc &TFilename, int TWidth, int THeight) {
	Init();

	Filename = TFilename;
	Width = TWidth;
	Height = THeight;

	AllocateMap();
}

// Constructor for the map editor: load map
_Map::_Map(const core::stringc &TFilename) {
	Init();

	Filename = TFilename;
}

// Constructor for maps already created in the database
_Map::_Map(int TMapID) {
	Init();

	// Set ID
	ID = TMapID;

	// Get map info
	const _MapStat *Map = Stats.GetMap(ID);
	ViewSize.Width = Map->ViewWidth;
	ViewSize.Height = Map->ViewHeight;

	// Load map
	Filename = Map->File;
	LoadMap();
}

// Destructor
_Map::~_Map() {

	// Delete map data
	FreeMap();
}

// Initialize variables
void _Map::Init() {
	ID = 0;
	NoZoneTexture = nullptr;
	//DefaultNoZoneTexture = irrDriver->getTexture("textures/editor/nozone.png");
	Tiles = nullptr;
	ViewSize.Width = 25;
	ViewSize.Height = 19;
	CameraScroll.X = CAMERA_SCROLLMIN_X;
	CameraScroll.Y = CAMERA_SCROLLMIN_Y;
}

// Free memory used by the tiles
void _Map::FreeMap() {
	if(Tiles) {
		for(int i = 0; i < Width; i++)
			delete[] Tiles[i];
		delete[] Tiles;

		Tiles = nullptr;
	}

	IndexedEvents.clear();
}

// Allocates memory for the map
void _Map::AllocateMap() {
	if(Tiles)
		return;

	Tiles = new _Tile*[Width];
	for(int i = 0; i < Width; i++) {
		Tiles[i] = new _Tile[Height];
	}

	// Delete textures
	//for(size_t i = 0; i < Textures.size(); i++)
	//	irrDriver->removeTexture(Textures[i]);
	Textures.clear();
}

// Updates the map and sends object updates
void _Map::Update(double FrameTime) {

	ObjectUpdateTime += FrameTime;
	if(ObjectUpdateTime > NETWORK_UPDATEPERIOD) {
		ObjectUpdateTime = 0;

		SendObjectUpdates();
	}
}

// Renders the map
void _Map::Render() {

	core::position2di GridPosition, DrawPosition;
	for(int i = 0; i < ViewSize.Width; i++) {
		for(int j = 0; j < ViewSize.Height; j++) {

			// Get the actual grid coordinate
			GridPosition.X = i + CameraScroll.X - ViewSize.Width / 2;
			GridPosition.Y = j + CameraScroll.Y - ViewSize.Height / 2;
			DrawPosition = core::position2di((i - ViewSize.Width / 2) * MAP_TILE_WIDTH + 400, (j - ViewSize.Height / 2) * MAP_TILE_HEIGHT + 300);
			if(NoZoneTexture)
				Graphics.DrawCenteredImage(NoZoneTexture, DrawPosition.X, DrawPosition.Y);

			// Validate coordinate
			if(GridPosition.X >= 0 && GridPosition.X < Width && GridPosition.Y >= 0 && GridPosition.Y < Height) {
				_Tile *Tile = &Tiles[GridPosition.X][GridPosition.Y];

				if(Tile->Texture)
					Graphics.DrawCenteredImage(Tile->Texture, DrawPosition.X, DrawPosition.Y);
			}
		}
	}
}

// Renders the map for editor
void _Map::RenderForMapEditor(bool TDrawWall, bool TDrawZone, bool TDrawPVP) {

	core::position2di GridPosition, DrawPosition;
	for(int i = 0; i < ViewSize.Width; i++) {
		for(int j = 0; j < ViewSize.Height; j++) {

			// Get the actual grid coordinate
			GridPosition.X = i + CameraScroll.X - ViewSize.Width / 2;
			GridPosition.Y = j + CameraScroll.Y - ViewSize.Height / 2;
			DrawPosition = core::position2di((i - ViewSize.Width / 2) * MAP_TILE_WIDTH + 400, (j - ViewSize.Height / 2) * MAP_TILE_HEIGHT + 300);
			if(NoZoneTexture)
				Graphics.DrawCenteredImage(NoZoneTexture, DrawPosition.X, DrawPosition.Y);

			// Validate coordinate
			if(GridPosition.X >= 0 && GridPosition.X < Width && GridPosition.Y >= 0 && GridPosition.Y < Height) {
				_Tile *Tile = &Tiles[GridPosition.X][GridPosition.Y];

				// Draw texture
				if(Tile->Texture)
					Graphics.DrawCenteredImage(Tile->Texture, DrawPosition.X, DrawPosition.Y);
				else if(NoZoneTexture)
					Graphics.DrawCenteredImage(NoZoneTexture, DrawPosition.X, DrawPosition.Y);

				// Draw wall
				if(TDrawWall && Tile->Wall)
					Graphics.RenderText("W", DrawPosition.X, DrawPosition.Y - 8, _Graphics::ALIGN_CENTER);

				// Draw zone
				if(!Tile->Wall) {
					if(TDrawZone && Tile->Zone > 0)
						Graphics.RenderText(core::stringc(Tile->Zone).c_str(), DrawPosition.X, DrawPosition.Y - 8, _Graphics::ALIGN_CENTER);

					// Draw PVP
					if(TDrawPVP && Tile->PVP)
						Graphics.RenderText("PvP", DrawPosition.X, DrawPosition.Y - 8, _Graphics::ALIGN_CENTER, video::SColor(255, 255, 0, 0));
				}

				// Draw event info
				if(Tile->EventType > 0) {
					core::stringc EventText = Stats.GetEvent(Tile->EventType)->ShortName + core::stringc(", ") + core::stringc(Tile->EventData);
					Graphics.RenderText(EventText.c_str(), DrawPosition.X - 16, DrawPosition.Y - 16, _Graphics::ALIGN_LEFT, video::SColor(255, 0, 255, 255));
				}
			}
			else {
				Graphics.DrawCenteredImage(DefaultNoZoneTexture, DrawPosition.X, DrawPosition.Y);
			}
		}
	}
}

// Sets the camera scroll position
void _Map::SetCameraScroll(const core::position2di &TPosition) {

	CameraScroll = TPosition;
	if(CameraScroll.X < CAMERA_SCROLLMIN_X)
		CameraScroll.X = CAMERA_SCROLLMIN_X;
	if(CameraScroll.Y < CAMERA_SCROLLMIN_Y)
		CameraScroll.Y = CAMERA_SCROLLMIN_Y;
	if(CameraScroll.X >= Width - CAMERA_SCROLLMIN_X)
		CameraScroll.X = Width - CAMERA_SCROLLMIN_X;
	if(CameraScroll.Y >= Height - CAMERA_SCROLLMIN_Y)
		CameraScroll.Y = Height - CAMERA_SCROLLMIN_Y;
}

// Converts a grid position on the map to a screen coordinate
bool _Map::GridToScreen(const core::position2di &TGridPosition, core::position2di &TScreenPosition) const {

	// Get delta from center
	core::position2di CenterDelta(TGridPosition.X - CameraScroll.X, TGridPosition.Y - CameraScroll.Y);

	TScreenPosition.X = CenterDelta.X * MAP_TILE_WIDTH + 400;
	TScreenPosition.Y = CenterDelta.Y * MAP_TILE_HEIGHT + 300;

	// Check if it's on screen
	if(abs(CenterDelta.X) > ViewSize.Width/2 || abs(CenterDelta.Y) > ViewSize.Height/2)
		return false;

	return true;
}

// Converts a screen coordinate to a map position
void _Map::ScreenToGrid(const core::position2di &TScreenPosition, core::position2di &TGridPosition) const {
	TGridPosition.X = GetCameraScroll().X + TScreenPosition.X / MAP_TILE_WIDTH - GetViewSize().Width / 2;
	TGridPosition.Y = GetCameraScroll().Y + TScreenPosition.Y / MAP_TILE_HEIGHT - GetViewSize().Height / 2;
}

// Saves the map to a file
int _Map::SaveMap() {

	// Open file
	std::ofstream File(Filename.c_str(), std::ios::binary);
	if(!File) {
		printf("SaveMap: unable to open file for writing\n");
		return 0;
	}

	// Generate a list of textures used by the map
	std::vector<_Texture *> TextureList;
	GetTextureListFromMap(TextureList);

	// Write header
	File.write((char *)&MAP_VERSION, sizeof(MAP_VERSION));
	File.write((char *)&Width, sizeof(Width));
	File.write((char *)&Height, sizeof(Height));

	// Write texture list
	int32_t TextureCount = (int32_t)(TextureList.size());
	File.write((char *)&TextureCount, sizeof(TextureCount));
	for(int32_t i = 0; i < TextureCount; i++) {
		if(TextureList[i] == nullptr) {
			File.write("none", 4);
			File.put(0);
		}
		else {

			// Strip path from texture name
			//std::string TexturePath = TextureList[i]->Identifier;
			//int SlashIndex = TexturePath.findLastChar("/\\", 2);
			//TexturePath = TexturePath.subString(SlashIndex + 1, TexturePath.size() - SlashIndex - 1);

			//File.write(TexturePath.c_str(), TexturePath.size());
			//File.put(0);
		}
	}

	// Write no-zone texture
	if(NoZoneTexture == nullptr) {
		File.write("none", 4);
		File.put(0);
	}
	else {

		// Strip path from texture name
		//std::string TexturePath = NoZoneTexture->Identifier;
		//int SlashIndex = TexturePath.findLastChar("/\\", 2);
		//TexturePath = TexturePath.subString(SlashIndex + 1, TexturePath.size() - SlashIndex - 1);

		//File.write(TexturePath.c_str(), TexturePath.size());
		//File.put(0);
	}

	// Write map data
	_Tile *Tile;
	for(int i = 0; i < Width; i++) {
		for(int j = 0; j < Height; j++) {
			Tile = &Tiles[i][j];

			// Write texture
			int32_t TextureIndex = GetTextureIndex(TextureList, Tile->Texture);
			File.write((char *)&TextureIndex, sizeof(TextureIndex));
			File.write((char *)&Tile->Zone, sizeof(Tile->Zone));
			File.write((char *)&Tile->EventType, sizeof(Tile->EventType));
			File.write((char *)&Tile->EventData, sizeof(Tile->EventData));
			File.write((char *)&Tile->Wall, sizeof(Tile->Wall));
			File.write((char *)&Tile->PVP, sizeof(Tile->PVP));
		}
	}

	// Close file
	File.close();

	return 1;
}

// Loads a map
int _Map::LoadMap() {

	// Open file
	std::ifstream File(Filename.c_str(), std::ios::binary);
	if(!File) {
		printf("LoadMap: unable to open file for reading: %s\n", Filename.c_str());
		return 0;
	}

	// Read header
	int32_t MapVersion;
	File.read((char *)&MapVersion, sizeof(MapVersion));
	File.read((char *)&Width, sizeof(Width));
	File.read((char *)&Height, sizeof(Height));
	if(Width < 5 || Width > 255 || Height < 5 || Height > 255 || MapVersion != MAP_VERSION) {
		printf("LoadMap: bad size header\n");
		return 0;
	}

	// Allocate memory
	FreeMap();
	AllocateMap();

	// Get count of textures
	int32_t TextureCount;
	File.read((char *)&TextureCount, sizeof(TextureCount));
	Textures.clear();

	// Read textures from map
	core::stringc TextureFile;
	char String[256];
	for(int32_t i = 0; i < TextureCount; i++) {
		File.get(String, std::numeric_limits<std::streamsize>::max(), 0);
		File.get();

		TextureFile = String;
		//if(TextureFile == "none")
		//	Textures.push_back(nullptr);
		//else
		//	Textures.push_back(irrDriver->getTexture(core::stringc("textures/map/") + TextureFile));
	}

	// Get no zone texture
	File.get(String, std::numeric_limits<std::streamsize>::max(), 0);
	File.get();
	TextureFile = String;
	//if(TextureFile == "none")
	//	NoZoneTexture = nullptr;
	//else
	//	NoZoneTexture = irrDriver->getTexture(core::stringc("textures/map/") + TextureFile);

	// Read map data
	_Tile *Tile;
	for(int i = 0; i < Width; i++) {
		for(int j = 0; j < Height; j++) {
			Tile = &Tiles[i][j];

			int32_t TextureIndex;
			File.read((char *)&TextureIndex, sizeof(int32_t));
			Tile->Texture = Textures[TextureIndex];

			File.read((char *)&Tile->Zone, sizeof(Tile->Zone));
			File.read((char *)&Tile->EventType, sizeof(Tile->EventType));
			File.read((char *)&Tile->EventData, sizeof(Tile->EventData));
			File.read((char *)&Tile->Wall, sizeof(Tile->Wall));
			File.read((char *)&Tile->PVP, sizeof(Tile->PVP));

			// Save off events that need to be indexed
			if(Stats.GetEvent(Tile->EventType)->Indexed) {
				IndexedEvents.push_back(_IndexedEvent(Tile, core::position2di(i, j)));
			}
		}
	}

	// Close file
	File.close();

	return 1;
}

// Builds an array of textures that are used in the map
void _Map::GetTextureListFromMap(std::vector<_Texture *> &TTextures) {

	TTextures.clear();

	// Go through map
	for(int i = 0; i < Width; i++) {
		for(int j = 0; j < Height; j++) {

			// Check for new textures
			if(GetTextureIndex(TTextures, Tiles[i][j].Texture) == -1) {
				TTextures.push_back(Tiles[i][j].Texture);
			}
		}
	}
}

// Returns the index of a texture in an array
int _Map::GetTextureIndex(std::vector<_Texture *> &TTextures, _Texture *TTexture) {

	for(size_t i = 0; i < TTextures.size(); i++) {
		if(TTextures[i] == TTexture)
			return (int)i;
	}

	return -1;
}

// Determines if a square can be moved to
bool _Map::CanMoveTo(const core::position2di &TPosition) {

	// Bounds
	if(TPosition.X < 0 || TPosition.X >= Width || TPosition.Y < 0 || TPosition.Y >= Height)
		return false;

	return !Tiles[TPosition.X][TPosition.Y].Wall;
}

// Adds an object to the map
void _Map::AddObject(_Object *TObject) {

	// Create packet for the new object
	_Buffer Packet;
	Packet.Write<char>(_Network::WORLD_CREATEOBJECT);
	Packet.Write<char>(TObject->GetNetworkID());
	Packet.Write<char>(TObject->GetPosition().X);
	Packet.Write<char>(TObject->GetPosition().Y);
	Packet.Write<char>(TObject->GetType());
	switch(TObject->GetType()) {
		case _Object::PLAYER: {
			_Player *NewPlayer = static_cast<_Player *>(TObject);
			Packet.WriteString(NewPlayer->GetName().c_str());
			Packet.Write<char>(NewPlayer->GetPortraitID());
			Packet.WriteBit(NewPlayer->IsInvisible());
		}
		break;
		default:
		break;
	}

	// Notify other players of the new object
	SendPacketToPlayers(&Packet);

	// Add object to map
	Objects.push_back(TObject);
}

// Removes an object from the map
void _Map::RemoveObject(_Object *TObject) {

	// Remove from the map
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ) {
		if(*Iterator == TObject)
			Iterator = Objects.erase(Iterator);
		else
			++Iterator;
	}

	// Create delete packet
	_Buffer Packet;
	Packet.Write<char>(_Network::WORLD_DELETEOBJECT);
	Packet.Write<char>(TObject->GetNetworkID());

	// Send to everyone
	SendPacketToPlayers(&Packet);
}

// Returns the list of objects
const std::list<_Object *> &_Map::GetObjects() const {

	return Objects;
}

// Returns a list of players close to a player
void _Map::GetClosePlayers(const _Player *TPlayer, float TDistanceSquared, std::list<_Player *> &TPlayers) {

	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetType() == _Object::PLAYER) {
			_Player *Player = static_cast<_Player *>(*Iterator);
			if(Player != TPlayer) {
				int XDelta = Player->GetPosition().X - TPlayer->GetPosition().X;
				int YDelta = Player->GetPosition().Y - TPlayer->GetPosition().Y;
				if((float)(XDelta * XDelta + YDelta * YDelta) <= TDistanceSquared) {
					TPlayers.push_back(Player);
				}
			}
		}
	}
}

// Returns the closest player
_Player *_Map::GetClosestPlayer(const _Player *TPlayer, float TMaxDistanceSquared, int TState) {

	_Player *ClosestPlayer = nullptr;
	float ClosestDistanceSquared = 1e10;
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetType() == _Object::PLAYER) {
			_Player *Player = static_cast<_Player *>(*Iterator);
			if(Player != TPlayer && Player->GetState() == TState) {
				int XDelta = Player->GetPosition().X - TPlayer->GetPosition().X;
				int YDelta = Player->GetPosition().Y - TPlayer->GetPosition().Y;
				float DistanceSquared = (float)(XDelta * XDelta + YDelta * YDelta);
				if(DistanceSquared <= TMaxDistanceSquared && DistanceSquared < ClosestDistanceSquared) {
					ClosestDistanceSquared = DistanceSquared;
					ClosestPlayer = Player;
				}
			}
		}
	}

	return ClosestPlayer;
}

// Sends object position information to all the clients in the map
void _Map::SendObjectUpdates() {

	// unsequenced
	_Buffer Packet;
	Packet.Write<char>(_Network::WORLD_OBJECTUPDATES);

	// Get object count
	int ObjectCount = Objects.size();
	Packet.Write<char>(ObjectCount);

	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		_Object *Object = *Iterator;
		int State = 0;
		bool Invisible = false;
		if(Object->GetType() == _Object::PLAYER) {
			_Player *Player = static_cast<_Player *>(Object);
			State = Player->GetState();
			Invisible = Player->IsInvisible();
		}

		Packet.Write<char>(Object->GetNetworkID());
		Packet.Write<char>(State);
		Packet.Write<char>(Object->GetPosition().X);
		Packet.Write<char>(Object->GetPosition().Y);
		Packet.WriteBit(Invisible);
	}

	SendPacketToPlayers(&Packet, nullptr, _Network::UNSEQUENCED);
}

// Sends a packet to all of the players in the map
void _Map::SendPacketToPlayers(_Buffer *TPacket, _Player *ExceptionPlayer, _Network::SendType Type) {

	// Send the packet out
	for(std::list<_Object *>::iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
		if((*Iterator)->GetType() == _Object::PLAYER) {
			_Player *Player = static_cast<_Player *>(*Iterator);

			if(Player != ExceptionPlayer)
				ServerNetwork->SendPacketToPeer(TPacket, Player->GetPeer(), Type, Type == _Network::UNSEQUENCED);
		}
	}
}

// Finds an event that matches the criteria
_IndexedEvent *_Map::GetIndexedEvent(int TEventType, int TEventData) {

	for(size_t i = 0; i < IndexedEvents.size(); i++) {
		_IndexedEvent *IndexedEvent = &IndexedEvents[i];
		if(IndexedEvent->Tile->EventType == TEventType && IndexedEvent->Tile->EventData == TEventData) {
			return IndexedEvent;
		}
	}

	return nullptr;
}
