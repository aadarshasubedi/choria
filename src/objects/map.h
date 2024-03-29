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
#include <objects/managerbase.h>
#include <network/network.h>
#include <packet.h>
#include <texture.h>
#include <manager.h>
#include <vector>
#include <list>
#include <cstdint>
#include <string>
#include <path/micropather.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/detail/func_common.hpp>
#include <unordered_map>
#include <map>
#include <sstream>

// Forward Declarations
class _Object;
class _Buffer;
class _Atlas;
class _Camera;
class _Server;
class _Stats;
class _Peer;

// Structures
struct _Event {
	_Event() : Type(0), Data(0) { }
	_Event(uint32_t Type, uint32_t Data) : Type(Type), Data(Data) { }

	bool operator==(const _Event &Event) const { return Event.Type == Type && Event.Data == Data; }
	bool operator<(const _Event &Event) const { return std::tie(Event.Type, Event.Data) < std::tie(Type, Data); }

	uint32_t Type;
	uint32_t Data;
};

struct _Tile {
	_Tile() : TextureIndex{0, 0}, Zone(0), Wall(false), PVP(false) { }
	uint32_t TextureIndex[2];
	uint32_t Zone;
	_Event Event;
	bool Wall;
	bool PVP;
};

// Classes
class _Map : public _ManagerBase, public micropather::Graph {

	public:

		enum EventType {
			EVENT_NONE,
			EVENT_SPAWN,
			EVENT_MAPENTRANCE,
			EVENT_MAPCHANGE,
			EVENT_VENDOR,
			EVENT_TRADER,
			EVENT_KEY,
			EVENT_SCRIPT,
			EVENT_PORTAL,
			EVENT_JUMP,
			EVENT_COUNT
		};

		_Map();
		~_Map();

		void AllocateMap();
		void ResizeMap(glm::ivec2 Offset, glm::ivec2 NewSize);
		void InitAtlas(const std::string AtlasPath, bool Static=false);
		void CloseAtlas();

		void Update(double FrameTime) override;

		// Events
		void CheckEvents(_Object *Object);
		void IndexEvents();
		void GetClockAsString(std::stringstream &Buffer);
		void SetAmbientLightByClock();

		// Graphics
		void Render(_Camera *Camera, _Stats *Stats, _Object *ClientPlayer, double BlendFactor, int RenderFlags=0);
		void RenderLayer(const std::string &Program, glm::vec4 &Bounds, const glm::vec3 &Offset, int Layer, bool Static=false);

		// Collision
		bool CanMoveTo(const glm::ivec2 &Position, _Object *Object);

		// Peer management
		void BroadcastPacket(_Buffer &Buffer, _Network::SendType Type=_Network::RELIABLE);

		// Object management
		void SendObjectUpdates();
		void AddObject(_Object *Object);
		void RemoveObject(const _Object *RemoveObject);
		void SendObjectList(_Peer *Peer);
		void GetClosePlayers(const _Object *Player, float DistanceSquared, size_t Max, std::list<_Object *> &Players);
		_Object *FindTradePlayer(const _Object *Player, float MaxDistanceSquared);
		bool FindEvent(const _Event &Event, glm::ivec2 &Position);

		// Map editing
		bool IsValidPosition(const glm::ivec2 &Position) const { return Position.x >= 0 && Position.y >= 0 && Position.x < Size.x && Position.y < Size.y; }
		glm::vec2 GetValidPosition(const glm::vec2 &Position);
		glm::ivec2 GetValidCoord(const glm::ivec2 &Position);

		void GetTile(const glm::ivec2 &Position, _Tile &Tile) const { Tile = Tiles[Position.x][Position.y]; }
		const _Tile *GetTile(const glm::ivec2 &Position) const { return &Tiles[Position.x][Position.y]; }
		void SetTile(const glm::ivec2 &Position, const _Tile *Tile) { Tiles[Position.x][Position.y] = *Tile; }

		// File IO
		void Load(const std::string &Path, bool Static=false);
		bool Save(const std::string &Path);

		void NodeToPosition(void *Node, glm::ivec2 &Position) {
			int Index = (int)(intptr_t)Node;
			Position.y = Index / Size.x;
			Position.x = Index - Position.y * Size.x;
		}

		void *PositionToNode(const glm::ivec2 &Position) { return (void *)(intptr_t)(Position.y * Size.x + Position.x); }

		// Map data
		_Tile **Tiles;
		glm::ivec2 Size;
		std::map<_Event, glm::ivec2> IndexedEvents;

		// Graphics
		bool UseAtlas;
		const _Atlas *TileAtlas;
		glm::vec4 AmbientLight;
		int IsOutside;
		double Clock;

		// Background
		std::string BackgroundMapFile;
		glm::vec3 BackgroundOffset;
		_Map *BackgroundMap;

		// Objects
		std::list<_Object *> Objects;
		double ObjectUpdateTime;

		// Network
		_Server *Server;

	private:

		void FreeMap();

		// Path finding
		float LeastCostEstimate(void *StateStart, void *StateEnd) override;
		void AdjacentCost(void *State, std::vector<micropather::StateCost> *Neighbors) override;
		void PrintStateInfo(void *State) override { }

		// Rendering
		uint32_t TileVertexBufferID[2];
		uint32_t TileElementBufferID;
		glm::vec4 *TileVertices[2];
		glm::u32vec3 *TileFaces;

		// Network
		std::list<const _Peer *> Peers;

};
