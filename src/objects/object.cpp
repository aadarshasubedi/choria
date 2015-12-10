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
#include <objects/object.h>
#include <objects/skill.h>
#include <instances/map.h>
#include <instances/battle.h>
#include <ui/element.h>
#include <ui/image.h>
#include <buffer.h>
#include <assets.h>
#include <graphics.h>
#include <random.h>
#include <stats.h>
#include <font.h>
#include <scripting.h>
#include <program.h>
#include <constants.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <iostream>

// Constructor
_Object::_Object()
:	Map(nullptr),
	Peer(nullptr),
	Type(0),
	InputState(0),
	Moved(false),
	Deleted(false),
	WaitForServer(false),
	Position(0, 0),
	ServerPosition(0, 0),
	NetworkID(0),

	Name(""),
	Level(0),
	Health(0),
	MaxHealth(0),
	Mana(0),
	MaxMana(0),
	MinDamage(0),
	MaxDamage(0),
	MinDefense(0),
	MaxDefense(0),
	HealthRegen(0.0f),
	ManaRegen(0.0f),
	HealthAccumulator(0.0f),
	ManaAccumulator(0.0f),
	Battle(nullptr),
	BattleSpeed(BATTLE_DEFAULTSPEED),
	TurnTimer(0.0),
	AITimer(1.0),
	BattleID(-1),
	BattleSide(0),
	Portrait(nullptr),
	BattleOffset(0, 0),

	DatabaseID(0),
	ExperienceGiven(0),
	GoldGiven(0),
	AI(""),

	CharacterID(0),
	CheckEvent(false),
	Paused(false),
	MoveTime(0),
	Status(0),
	PortraitID(0),
	WorldTexture(nullptr),
	StatusTexture(nullptr),
	SpawnMapID(1),
	SpawnPoint(0),
	TeleportTime(-1),
	PlayTime(0),
	PlayTimeAccumulator(0),
	Deaths(0),
	MonsterKills(0),
	PlayerKills(0),
	Bounty(0),
	Gold(0),
	Experience(0),
	ExperienceNeeded(0),
	ExperienceNextLevel(0),
	MinDamageBonus(0),
	MaxDamageBonus(0),
	MinDefenseBonus(0),
	MaxDefenseBonus(0),
	WeaponDamageModifier(0.0f),
	WeaponMinDamage(0),
	WeaponMaxDamage(0),
	ArmorMinDefense(0),
	ArmorMaxDefense(0),
	NextBattle(0),
	AttackPlayerTime(0),
	InvisPower(0),
	InventoryOpen(false),
	Vendor(nullptr),
	Trader(nullptr),
	SkillsOpen(false),
	SkillPoints(0),
	SkillPointsUsed(0),
	TradeGold(0),
	WaitingForTrade(false),
	TradeAccepted(false),
	TradePlayer(nullptr),
	Stats(nullptr) {

	for(int i = 0; i < _Object::INVENTORY_COUNT; i++) {
		Inventory[i].Item = nullptr;
		Inventory[i].Count = 0;
	}
}

// Destructor
_Object::~_Object() {
}

// Renders the fighter during a battle
void _Object::RenderBattle(_Object *ClientPlayer, double Time) {

	// Get slot ui element depending on side
	_Element *Slot;
	if(BattleSide == 0)
		Slot = Assets.Elements["element_side_left"];
	else
		Slot = Assets.Elements["element_side_right"];

	// Draw slot
	Slot->Offset = BattleOffset;
	Slot->CalculateBounds();
	Slot->SetVisible(true);
	Slot->Render();
	Slot->SetVisible(false);

	// Get slot center
	glm::ivec2 SlotPosition = (Slot->Bounds.Start + Slot->Bounds.End) / 2;

	// Name
	Assets.Fonts["hud_medium"]->DrawText(Name.c_str(), Slot->Bounds.Start + glm::ivec2(32, -10), COLOR_WHITE, CENTER_BASELINE);

	// Portrait
	if(Portrait) {
		Graphics.SetProgram(Assets.Programs["ortho_pos_uv"]);
		Graphics.DrawCenteredImage(SlotPosition, Portrait);
	}

	// Health/mana bars
	glm::ivec2 BarSize(90, 22);
	glm::ivec2 BarOffset(Slot->Size.x/2 + 10, -Slot->Size.y/2);
	float BarPaddingY = 6;

	// Get health percent
	float HealthPercent = MaxHealth > 0 ? Health / (float)MaxHealth : 0;

	// Get ui size
	_Bounds BarBounds;
	BarBounds.Start = SlotPosition + glm::ivec2(0, 0) + BarOffset;
	BarBounds.End = SlotPosition + glm::ivec2(BarSize.x, BarSize.y) + BarOffset;
	glm::ivec2 BarCenter = (BarBounds.Start + BarBounds.End) / 2;
	//glm::ivec2 HealthBarCenter = BarCenter;
	ResultPosition = SlotPosition;
	int BarEndX = BarBounds.End.x;

	// Draw empty bar
	Graphics.SetProgram(Assets.Programs["ortho_pos_uv"]);
	Graphics.SetVBO(VBO_NONE);
	Graphics.DrawImage(BarBounds, Assets.Images["image_hud_health_bar_empty"]->Texture, true);

	// Draw full bar
	BarBounds.End = SlotPosition + glm::ivec2(BarSize.x * HealthPercent, BarSize.y) + BarOffset;
	Graphics.DrawImage(BarBounds, Assets.Images["image_hud_health_bar_full"]->Texture, true);

	// Draw health text
	std::stringstream Buffer;
	Buffer << Health << " / " << MaxHealth;
	Assets.Fonts["hud_small"]->DrawText(Buffer.str().c_str(), BarCenter + glm::ivec2(0, 5), COLOR_WHITE, CENTER_BASELINE);
	Buffer.str("");

	// Draw mana
	if(MaxMana > 0) {
		float ManaPercent = MaxMana > 0 ? Mana / (float)MaxMana : 0;

		// Get ui size
		BarOffset.y += BarSize.y + BarPaddingY;
		BarBounds.Start = SlotPosition + glm::ivec2(0, 0) + BarOffset;
		BarBounds.End = SlotPosition + glm::ivec2(BarSize.x, BarSize.y) + BarOffset;
		BarCenter = (BarBounds.Start + BarBounds.End) / 2;

		// Draw empty bar
		Graphics.SetProgram(Assets.Programs["ortho_pos_uv"]);
		Graphics.SetVBO(VBO_NONE);
		Graphics.DrawImage(BarBounds, Assets.Images["image_hud_mana_bar_empty"]->Texture, true);

		// Draw full bar
		BarBounds.End = SlotPosition + glm::ivec2(BarSize.x * ManaPercent, BarSize.y) + BarOffset;
		Graphics.DrawImage(BarBounds, Assets.Images["image_hud_mana_bar_full"]->Texture, true);

		// Draw mana text
		Buffer << Mana << " / " << MaxMana;
		Assets.Fonts["hud_small"]->DrawText(Buffer.str().c_str(), BarCenter + glm::ivec2(0, 5), COLOR_WHITE, CENTER_BASELINE);
		Buffer.str("");
	}

	// Draw turn timer
	BarOffset.y += BarSize.y + BarPaddingY;
	BarSize.y = 8;
	BarBounds.Start = SlotPosition + glm::ivec2(0, 0) + BarOffset;
	BarBounds.End = SlotPosition + glm::ivec2(BarSize.x, BarSize.y) + BarOffset;

	// Draw empty bar
	Graphics.SetProgram(Assets.Programs["ortho_pos_uv"]);
	Graphics.SetVBO(VBO_NONE);
	Graphics.DrawImage(BarBounds, Assets.Images["image_hud_experience_bar_empty"]->Texture, true);

	// Draw full bar
	BarBounds.End = SlotPosition + glm::ivec2(BarSize.x * TurnTimer, BarSize.y) + BarOffset;
	Graphics.DrawImage(BarBounds, Assets.Images["image_hud_experience_bar_full"]->Texture, true);

	// Draw the skill used
	if(ClientPlayer->BattleSide == BattleSide && BattleAction.Skill) {
		glm::ivec2 SkillUsingPosition = SlotPosition - glm::ivec2(Portrait->Size.x/2 + BattleAction.Skill->Texture->Size.x/2 + 10, 0);
		Graphics.SetProgram(Assets.Programs["ortho_pos_uv"]);
		Graphics.DrawCenteredImage(SkillUsingPosition, BattleAction.Skill->Texture);
	}

	// Draw potential skill to use
	for(auto &BattleTarget : ClientPlayer->BattleTargets) {
		if(BattleTarget == this && ClientPlayer->PotentialAction.IsSet()) {
			const _Texture *Texture = nullptr;
			if(ClientPlayer->PotentialAction.Skill)
				Texture = ClientPlayer->PotentialAction.Skill->Texture;
			else if(ClientPlayer->PotentialAction.Item)
				Texture = ClientPlayer->PotentialAction.Item->Image;

			Graphics.SetProgram(Assets.Programs["ortho_pos_uv"]);
			glm::vec4 Color(COLOR_WHITE);
			if(Time - (int)Time < 0.5f)
				Color.a = 0.5f;

			Graphics.DrawCenteredImage(glm::ivec2(BarEndX + Texture->Size.x/2 + 10, SlotPosition.y), Texture, Color);
		}
	}
}

// Update health
void _Object::UpdateHealth(int Value) {
	Health += Value;

	if(Health < 0)
		Health = 0;
	else if(Health > MaxHealth)
		Health = MaxHealth;
}

// Update mana
void _Object::UpdateMana(int Value) {
	Mana += Value;

	if(Mana < 0)
		Mana = 0;
	else if(Mana > MaxMana)
		Mana = MaxMana;
}

// Set health and mana to max
void _Object::RestoreHealthMana() {
	Health = MaxHealth;
	Mana = MaxMana;
}

// Updates the fighter's regen
void _Object::UpdateRegen(int &HealthUpdate, int &ManaUpdate) {

	HealthUpdate = 0;
	ManaUpdate = 0;
	HealthAccumulator += HealthRegen * 0.01f * MaxHealth;
	ManaAccumulator += ManaRegen * 0.01f * MaxMana;

	if(HealthAccumulator >= 1.0f) {
		int IntegerAccumulator = (int)HealthAccumulator;

		HealthUpdate = IntegerAccumulator;
		HealthAccumulator -= IntegerAccumulator;
	}
	else if(HealthAccumulator < 0.0f) {
		HealthAccumulator = 0;
	}

	if(ManaAccumulator >= 1.0f) {
		int IntegerAccumulator = (int)ManaAccumulator;

		ManaUpdate = IntegerAccumulator;
		ManaAccumulator -= IntegerAccumulator;
	}
	else if(ManaAccumulator < 0.0f) {
		ManaAccumulator = 0;
	}
}

// Update AI during battle
void _Object::UpdateAI(_Scripting *Scripting, const std::list<_Object *> &Fighters, double FrameTime) {
	if(!AI.length())
		return;

	// Update AI every second
	AITimer += FrameTime;
	if(AITimer >= BATTLE_AI_UPDATE_PERIOD) {
		AITimer = 0.0;

		// Separate fighter list
		std::list<_Object *> Enemies, Allies;
		for(const auto &Fighter : Fighters) {
			if(Fighter->BattleSide == BattleSide)
				Allies.push_back(Fighter);
			else if(Fighter->Health > 0)
				Enemies.push_back(Fighter);
		}

		// Call lua script
		if(Enemies.size()) {
			Scripting->StartMethodCall(AI, "Update");
			Scripting->PushObject(this);
			Scripting->PushObjectList(Enemies);
			Scripting->PushObjectList(Allies);
			Scripting->MethodCall(3, 0);
			Scripting->FinishMethodCall();
		}
	}
}

// Generate damage
int _Object::GenerateDamage() {
	return GetRandomInt(MinDamage, MaxDamage);
}

// Generate defense
int _Object::GenerateDefense() {
	return GetRandomInt(MinDefense, MaxDefense);
}

// Updates the player
void _Object::Update(double FrameTime) {
	CheckEvent = false;

	// Update player position
	Moved = Move();
	if(Moved) {
		InputState = 0;
		CheckEvent = true;
	}

	Status = STATUS_NONE;
	if(Battle)
		Status = STATUS_BATTLE;
	else if(WaitingForTrade)
		Status = STATUS_TRADE;
	else if(Vendor)
		Status = STATUS_VENDOR;
	else if(Trader)
		Status = STATUS_TRADER;
	else if(InventoryOpen)
		Status = STATUS_INVENTORY;
	else if(SkillsOpen)
		Status = STATUS_SKILLS;
	else if(Paused)
		Status = STATUS_PAUSE;

	// Update timers
	MoveTime += FrameTime;
	AttackPlayerTime += FrameTime;

	// Update teleport time
	if(TeleportTime > 0.0) {
		Status = STATUS_TELEPORT;
		TeleportTime -= FrameTime;
		if(TeleportTime <= 0.0) {
			CheckEvent = true;
			TeleportTime = 0.0;
		}
	}

	// Update playtime
	PlayTimeAccumulator += FrameTime;
	if(PlayTimeAccumulator >= 1.0) {
		PlayTimeAccumulator -= 1.0;
		PlayTime++;
	}
}

// Serialize for ObjectCreate
void _Object::Serialize(_Buffer &Packet) {
	Packet.Write<NetworkIDType>(NetworkID);
	Packet.Write<glm::ivec2>(Position);
	Packet.Write<char>(Type);
	Packet.WriteString(Name.c_str());
	Packet.Write<uint32_t>(PortraitID);
	Packet.WriteBit(IsInvisible());
}

// Serialize for ObjectUpdate
void _Object::SerializeUpdate(_Buffer &Packet) {
	Packet.Write<NetworkIDType>(NetworkID);
	Packet.Write<glm::ivec2>(Position);
	Packet.Write<char>(Status);
	Packet.WriteBit(IsInvisible());
}

// Unserialize for ObjectCreate
void _Object::Unserialize(_Buffer &Packet) {
	Position = Packet.Read<glm::ivec2>();
	Type = Packet.Read<char>();
	Name = Packet.ReadString();
	PortraitID = Packet.Read<uint32_t>();
	Portrait = Stats->GetPortraitImage(PortraitID);
	InvisPower = Packet.ReadBit();
	WorldTexture = Assets.Textures["players/basic.png"];
}

// Renders the player while walking around the world
void _Object::Render(const _Object *ClientPlayer) {
	if(Map && WorldTexture) {

		float Alpha = 1.0f;
		if(IsInvisible())
			Alpha = PLAYER_INVIS_ALPHA;

		Graphics.SetProgram(Assets.Programs["pos_uv"]);
		glUniformMatrix4fv(Assets.Programs["pos_uv"]->ModelTransformID, 1, GL_FALSE, glm::value_ptr(glm::mat4(1)));

		Graphics.SetVBO(VBO_QUAD);

		glm::vec4 Color(1.0f, 1.0f, 1.0f, Alpha);

		glm::vec3 DrawPosition;
		if(0) {
			DrawPosition = glm::vec3(ServerPosition, 0.0f) + glm::vec3(0.5f, 0.5f, 0);
			Graphics.SetColor(glm::vec4(1, 0, 0, 1));
			Graphics.DrawSprite(DrawPosition, WorldTexture);
		}

		DrawPosition = glm::vec3(Position, 0.0f) + glm::vec3(0.5f, 0.5f, 0);
		Graphics.SetColor(Color);
		Graphics.DrawSprite(DrawPosition, WorldTexture);
		if(StatusTexture) {
			Graphics.DrawSprite(DrawPosition, StatusTexture);
		}

		if(ClientPlayer != this) {
			Assets.Fonts["hud_small"]->DrawText(Name.c_str(), glm::vec2(DrawPosition) + glm::vec2(0, -0.5f), Color, CENTER_BASELINE, 1.0f / WorldTexture->Size.x);
		}
	}
}

// Moves the player
int _Object::Move() {
	if(WaitForServer || InputState == 0 || Battle)
		return 0;

	// Get new position
	glm::ivec2 Direction(0, 0);
	if(InputState & MOVE_UP)
		Direction.y += -1;
	if(InputState & MOVE_DOWN)
		Direction.y += 1;
	if(InputState & MOVE_LEFT)
		Direction.x += -1;
	if(InputState & MOVE_RIGHT)
		Direction.x += 1;

	// Remove diagonols
	if(Direction.x != 0 && Direction.y != 0)
		Direction.x = 0;

	// Check timer
	if(MoveTime < PLAYER_MOVETIME)
		return 0;

	// Move player
	if(Map->CanMoveTo(Position + Direction)) {
		Position += Direction;
		if(InvisPower > 0)
			InvisPower--;
		else
			NextBattle--;

		// Update regen
		int HealthUpdate, ManaUpdate;
		UpdateRegen(HealthUpdate, ManaUpdate);
		UpdateHealth(HealthUpdate);
		UpdateMana(ManaUpdate);

		MoveTime = 0;

		return InputState;
	}

	return 0;
}

// Gets the tile that the player is currently standing on
const _Tile *_Object::GetTile() {

	return Map->GetTile(Position);
}

// Generates the number of moves until the next battle
void _Object::GenerateNextBattle() {
	NextBattle = GetRandomInt(BATTLE_MINSTEPS, BATTLE_MAXSTEPS);
	NextBattle = 1;
}

// Stop a battle
void _Object::StopBattle() {
	Battle = nullptr;
	GenerateNextBattle();
}

// Determines if a player can attack
bool _Object::CanAttackPlayer() {

	return false; //AttackPlayerTime > PLAYER_ATTACKTIME;
}

// Update gold amount
void _Object::UpdateGold(int Value) {

	Gold += Value;
	if(Gold < 0)
		Gold = 0;
	else if(Gold > STATS_MAXGOLD)
		Gold = STATS_MAXGOLD;
}

// Search for an item in the inventory
bool _Object::FindItem(const _Item *Item, int &Slot) {
	for(int i = INVENTORY_BACKPACK; i < INVENTORY_TRADE; i++) {
		if(Inventory[i].Item == Item) {
			Slot = i;
			return true;
		}
	}

	return false;
}

// Count the number of a certain item in inventory
int _Object::CountItem(const _Item *Item) {
	int Count = 0;
	for(int i = INVENTORY_BACKPACK; i < INVENTORY_TRADE; i++) {
		if(Inventory[i].Item == Item)
			Count += Inventory[i].Count;
	}

	return Count;
}

// Update counts on action bar
void _Object::RefreshActionBarCount() {
	for(size_t i = 0; i < ActionBar.size(); i++) {
		if(ActionBar[i].Item)
			ActionBar[i].Count = CountItem(ActionBar[i].Item);
		else
			ActionBar[i].Count = 0;
	}
}

// Use an action, return true if used
bool _Object::UseAction(uint8_t Slot) {
	if(Slot >= ActionBar.size())
		return false;

	if(!ActionBar[Slot].IsSet())
		return false;

	if(ActionBar[Slot].Skill) {

	}
	else if(ActionBar[Slot].Item) {
		int Index = -1;
		if(FindItem(ActionBar[Slot].Item, Index)) {
			if(UseInventory(Index))
				return true;
		}
	}

	return false;
}

// Get the percentage to the next level
float _Object::GetNextLevelPercent() const {
	float Percent = 0;

	if(ExperienceNextLevel > 0)
		Percent = 1.0f - (float)ExperienceNeeded / ExperienceNextLevel;

	return Percent;
}

// Fills an array with inventory indices correlating to a trader's required items
int _Object::GetRequiredItemSlots(int *Slots) {
	int RewardItemSlot = -1;

	// Check for an open reward slot
	for(int i = INVENTORY_BACKPACK; i < INVENTORY_TRADE; i++) {
		_InventorySlot *InventoryItem = &Inventory[i];
		if(InventoryItem->Item == nullptr || (InventoryItem->Item == Trader->RewardItem && InventoryItem->Count + Trader->Count <= 255)) {
			RewardItemSlot = i;
			break;
		}
	}

	// Go through required items
	for(size_t i = 0; i < Trader->TraderItems.size(); i++) {
		const _Item *RequiredItem = Trader->TraderItems[i].Item;
		int RequiredCount = Trader->TraderItems[i].Count;
		Slots[i] = -1;

		// Search for the required item
		for(int j = INVENTORY_HEAD; j < INVENTORY_TRADE; j++) {
			_InventorySlot *InventoryItem = &Inventory[j];
			if(InventoryItem->Item == RequiredItem && InventoryItem->Count >= RequiredCount) {
				Slots[i] = j;
				break;
			}
		}

		// Didn't find an item
		if(Slots[i] == -1)
			RewardItemSlot = -1;
	}

	return RewardItemSlot;
}

// Accept a trade from a trader
void _Object::AcceptTrader(int *Slots, int RewardSlot) {
	if(Trader == nullptr || !IsSlotInventory(RewardSlot))
		return;

	// Trade in required items
	for(uint32_t i = 0; i < Trader->TraderItems.size(); i++) {
		UpdateInventory(Slots[i], -Trader->TraderItems[i].Count);
	}

	// Give player reward
	AddItem(Trader->RewardItem, Trader->Count, RewardSlot);
}

// Uses a potion in the world
bool _Object::UsePotionWorld(int Slot) {
	const _Item *Item = GetInventoryItem(Slot);
	if(Item == nullptr)
		return false;

	// Get potion stats
	int HealthRestore = Item->HealthRestore;
	int ManaRestore = Item->ManaRestore;
	int ItemInvisPower = Item->InvisPower;

	// Use only if needed
	if((Item->IsHealthPotion() && Health < MaxHealth) || (Item->IsManaPotion() && Mana < MaxMana) || Item->IsInvisPotion()) {
		UpdateHealth(HealthRestore);
		UpdateMana(ManaRestore);
		InvisPower = ItemInvisPower;
		UpdateInventory(Slot, -1);
		return true;
	}

	return false;
}

// Uses an item from the inventory
bool _Object::UseInventory(int Slot) {
	const _Item *Item = GetInventoryItem(Slot);
	if(Item == nullptr)
		return false;

	// Handle item types
	bool Used = false;
	switch(Item->Type) {
		case _Item::TYPE_POTION:
			Used = UsePotionWorld(Slot);
		break;
	}

	// Update action bar counts
	if(Used)
		RefreshActionBarCount();

	return Used;
}

// Sets an item in the inventory
void _Object::SetInventory(int Slot, int ItemID, int Count) {

	if(ItemID == 0) {
		Inventory[Slot].Item = nullptr;
		Inventory[Slot].Count = 0;
	}
	else {
		Inventory[Slot].Item = Stats->Items[ItemID];
		Inventory[Slot].Count = Count;
	}
}

// Sets an item in the inventory
void _Object::SetInventory(int Slot, _InventorySlot *Item) {

	if(Item->Item == nullptr) {
		Inventory[Slot].Item = nullptr;
		Inventory[Slot].Count = 0;
	}
	else {
		Inventory[Slot].Item = Item->Item;
		Inventory[Slot].Count = Item->Count;
	}
}

// Gets an inventory item
const _Item *_Object::GetInventoryItem(int Slot) {

	// Check for bad slots
	if(Slot < INVENTORY_BACKPACK || Slot >= INVENTORY_TRADE || !Inventory[Slot].Item)
		return nullptr;

	return Inventory[Slot].Item;
}

// Moves an item from one slot to another
bool _Object::MoveInventory(int OldSlot, int NewSlot) {
	if(OldSlot == NewSlot)
		return false;

	// Equippable items
	if(NewSlot < INVENTORY_BACKPACK) {

		// Check if the item is even equippable
		if(!CanEquipItem(NewSlot, Inventory[OldSlot].Item))
			return false;

		// Split stacks
		if(Inventory[OldSlot].Count > 1) {
			Inventory[NewSlot].Item = Inventory[OldSlot].Item;
			Inventory[NewSlot].Count = 1;
			Inventory[OldSlot].Count--;
		}
		else
			SwapItem(NewSlot, OldSlot);

		return true;
	}
	// Back pack
	else {

		// Add to stack
		if(Inventory[NewSlot].Item == Inventory[OldSlot].Item) {
			Inventory[NewSlot].Count += Inventory[OldSlot].Count;

			// Group stacks
			if(Inventory[NewSlot].Count > 255) {
				Inventory[OldSlot].Count = Inventory[NewSlot].Count - 255;
				Inventory[NewSlot].Count = 255;
			}
			else
				Inventory[OldSlot].Item = nullptr;
		}
		else {

			// Disable reverse equip for now
			if(OldSlot < INVENTORY_BACKPACK && Inventory[NewSlot].Item)
				return false;

			SwapItem(NewSlot, OldSlot);
		}

		return true;
	}
}

// Swaps two items
void _Object::SwapItem(int Slot, int OldSlot) {
	_InventorySlot TempItem;

	// Swap items
	TempItem = Inventory[Slot];
	Inventory[Slot] = Inventory[OldSlot];
	Inventory[OldSlot] = TempItem;
}

// Updates an item's count, deleting if necessary
bool _Object::UpdateInventory(int Slot, int Amount) {

	Inventory[Slot].Count += Amount;
	if(Inventory[Slot].Count <= 0) {
		Inventory[Slot].Item = nullptr;
		Inventory[Slot].Count = 0;
		return true;
	}

	return false;
}

// Attempts to add an item to the inventory
bool _Object::AddItem(const _Item *Item, int Count, int Slot) {

	// Place somewhere in backpack
	if(Slot == -1) {

		// Find existing item
		int EmptySlot = -1;
		for(int i = INVENTORY_BACKPACK; i < INVENTORY_TRADE; i++) {
			if(Inventory[i].Item == Item && Inventory[i].Count + Count <= 255) {
				Inventory[i].Count += Count;
				return true;
			}

			// Keep track of the first empty slot
			if(Inventory[i].Item == nullptr && EmptySlot == -1)
				EmptySlot = i;
		}

		// Found an empty slot
		if(EmptySlot != -1) {
			Inventory[EmptySlot].Item = Item;
			Inventory[EmptySlot].Count = Count;
			return true;
		}

		return false;
	}
	// Trying to equip an item
	else if(Slot < INVENTORY_BACKPACK) {

		// Make sure it can be equipped
		if(!CanEquipItem(Slot, Item))
			return false;

		// Set item
		Inventory[Slot].Item = Item;
		Inventory[Slot].Count = Count;

		return true;
	}

	// Add item
	if(Inventory[Slot].Item == Item && Inventory[Slot].Count + Count <= 255) {
		Inventory[Slot].Count += Count;
		return true;
	}
	else if(Inventory[Slot].Item == nullptr) {
		Inventory[Slot].Item = Item;
		Inventory[Slot].Count = Count;
		return true;
	}

	return false;
}

// Moves the player's trade items to their backpack
void _Object::MoveTradeToInventory() {

	for(int i = INVENTORY_TRADE; i < INVENTORY_COUNT; i++) {
		if(Inventory[i].Item && AddItem(Inventory[i].Item, Inventory[i].Count, -1))
			Inventory[i].Item = nullptr;
	}
}

// Splits a stack
void _Object::SplitStack(int Slot, int Count) {
	if(Slot < 0 || Slot >= INVENTORY_COUNT)
		return;

	// Make sure stack is large enough
	_InventorySlot *SplitItem = &Inventory[Slot];
	if(SplitItem->Item && SplitItem->Count > Count) {

		// Find an empty slot or existing item
		int EmptySlot = Slot;
		_InventorySlot *Item;
		do {
			EmptySlot++;
			if(EmptySlot >= INVENTORY_TRADE)
				EmptySlot = INVENTORY_BACKPACK;

			Item = &Inventory[EmptySlot];
		} while(!(EmptySlot == Slot || Item->Item == nullptr || (Item->Item == SplitItem->Item && Item->Count <= 255 - Count)));

		// Split item
		if(EmptySlot != Slot) {
			SplitItem->Count -= Count;
			AddItem(SplitItem->Item, Count, EmptySlot);
		}
	}
}

// Determines if the player can accept movement keys held down
bool _Object::AcceptingMoveInput() {
	if(Battle)
		return false;

	if(WaitForServer)
		return false;

	if(Vendor)
		return false;

	if(Trader)
		return false;

	return true;
}

// Determines if the player's backpack is full
bool _Object::IsBackpackFull() {

	// Search backpack
	for(int i = INVENTORY_BACKPACK; i < INVENTORY_TRADE; i++) {
		if(Inventory[i].Item == nullptr)
			return false;
	}

	return true;
}

// Checks if an item can be equipped
bool _Object::CanEquipItem(int Slot, const _Item *Item) {

	// Already equipped
	if(Inventory[Slot].Item)
		return false;

	// Check type
	switch(Slot) {
		case INVENTORY_HEAD:
			if(Item->Type == _Item::TYPE_HEAD)
				return true;
		break;
		case INVENTORY_BODY:
			if(Item->Type == _Item::TYPE_BODY)
				return true;
		break;
		case INVENTORY_LEGS:
			if(Item->Type == _Item::TYPE_LEGS)
				return true;
		break;
		case INVENTORY_HAND1:
			if(Item->Type == _Item::TYPE_WEAPON1HAND)
				return true;
		break;
		case INVENTORY_HAND2:
			if(Item->Type == _Item::TYPE_SHIELD)
				return true;
		break;
		case INVENTORY_RING1:
		case INVENTORY_RING2:
			if(Item->Type == _Item::TYPE_RING)
				return true;
		break;
		default:
		break;
	}

	return false;
}

// Updates a skill level
void _Object::AdjustSkillLevel(uint32_t SkillID, int Adjust) {
	if(SkillID == 0)
		return;

	const _Skill *Skill = Stats->Skills[SkillID];
	if(Skill == nullptr)
		return;

	// Buying
	if(Adjust > 0) {
		if(GetSkillPointsRemaining() == 0 || SkillLevels[SkillID] >= 255)
			return;

		// Update level
		SkillLevels[SkillID] += Adjust;
		if(SkillLevels[SkillID] > 255)
			SkillLevels[SkillID] = 255;
	}
	else if(Adjust < 0) {
		if(SkillLevels[SkillID] == 0)
			return;

		// Update level
		SkillLevels[SkillID] += Adjust;
		if(SkillLevels[SkillID] < 0)
			SkillLevels[SkillID] = 0;

		// Update skill bar
		if(SkillLevels[SkillID] == 0) {
			for(size_t i = 0; i < ActionBar.size(); i++) {
				if(ActionBar[i].Skill == Skill) {
					ActionBar[i].Unset();
					break;
				}
			}
		}
	}
}

// Calculates the number of skill points used
void _Object::CalculateSkillPoints() {

	SkillPointsUsed = 0;
	for(const auto &SkillLevel : SkillLevels) {
		const _Skill *Skill = Stats->Skills[SkillLevel.first];
		if(Skill)
			SkillPointsUsed += SkillLevel.second;
	}
}

// Can enter battle
bool _Object::CanBattle() {
	return Status == STATUS_NONE && !IsInvisible();
}

// Calculates all of the player stats
void _Object::CalculateStats() {
	HealthRegen = ManaRegen = 0.0f;
	MinDamage = MaxDamage = MinDefense = MaxDefense = 0;
	MinDamageBonus = MaxDamageBonus = MinDefenseBonus = MaxDefenseBonus = 0;
	WeaponMinDamage = WeaponMaxDamage = 0;
	ArmorMinDefense = ArmorMaxDefense = 0;
	WeaponDamageModifier = 1.0f;

	// Get base stats
	CalculateLevelStats();

	// Get gear stats
	CalculateGearStats();

	// Get skill bonus
	CalculateSkillStats();

	// Combine all stats
	CalculateFinalStats();

	// Cap stats
	UpdateHealth(0);
	UpdateMana(0);

	RefreshActionBarCount();
}

// Calculates the base level stats
void _Object::CalculateLevelStats() {
	if(!Stats)
		return;

	// Cap min experience
	if(Experience < 0)
		Experience = 0;

	// Cap max experience
	const _Level *MaxLevelStat = Stats->GetLevel(Stats->GetMaxLevel());
	if(Experience > MaxLevelStat->Experience)
		Experience = MaxLevelStat->Experience;

	// Find current level
	const _Level *LevelStat = Stats->FindLevel(Experience);
	Level = LevelStat->Level;
	MaxHealth = LevelStat->Health;
	MaxMana = LevelStat->Mana;
	SkillPoints = LevelStat->SkillPoints;
	ExperienceNextLevel = LevelStat->NextLevel;
	if(Level == Stats->GetMaxLevel())
		ExperienceNeeded = 0;
	else
		ExperienceNeeded = LevelStat->NextLevel - (Experience - LevelStat->Experience);

}

// Calculates stats from equipped items
void _Object::CalculateGearStats() {

	// Get stats
	if(!Inventory[INVENTORY_HAND1].Item)
		WeaponMinDamage = WeaponMaxDamage = 1;

	// Check each item
	for(int i = 0; i < INVENTORY_BACKPACK; i++) {
		const _Item *Item = Inventory[i].Item;
		if(Item) {
			int Min, Max;

			// Add damage
			Item->GetDamageRange(Min, Max);
			WeaponMinDamage += Min;
			WeaponMaxDamage += Max;

			// Add defense
			Item->GetDefenseRange(Min, Max);
			ArmorMinDefense += Min;
			ArmorMaxDefense += Max;

			// Boosts
			MaxHealth += Item->MaxHealth;
			MaxMana += Item->MaxMana;
			HealthRegen += Item->HealthRegen;
			ManaRegen += Item->ManaRegen;
		}
	}
}

// Calculates skill bonuses
void _Object::CalculateSkillStats() {
/*
	// Go through each skill bar
	for(size_t i = 0; i < ActionBar.size(); i++) {
		const _Skill *Skill = ActionBar[i].Skill;
		if(Skill) {
			int Min, Max, MinRound, MaxRound;
			float MinFloat, MaxFloat;
			Skill->GetPowerRange(SkillLevels[Skill->ID], Min, Max);
			Skill->GetPowerRangeRound(SkillLevels[Skill->ID], MinRound, MaxRound);
			Skill->GetPowerRange(SkillLevels[Skill->ID], MinFloat, MaxFloat);

			switch(Skill->ID) {
				case 1:
					WeaponDamageModifier = MaxFloat;
				break;
				case 5:
					MaxHealth += MaxRound;
				break;
				case 6:
					MaxMana += MaxRound;
				break;
				case 8:
					HealthRegen += MaxFloat;
				break;
				case 9:
					ManaRegen += MaxFloat;
				break;
				case 10:
					MinDamageBonus += Max;
					MaxDamageBonus += Max;
				break;
				case 11:
					MaxDefenseBonus += Max;
				break;
			}
		}
	}
	*/
}

// Combine all stats
void _Object::CalculateFinalStats() {
	MinDamage = MinDamageBonus + (int)std::roundf(WeaponMinDamage * WeaponDamageModifier);
	MaxDamage = MaxDamageBonus + (int)std::roundf(WeaponMaxDamage * WeaponDamageModifier);
	if(MinDamage < 0)
		MinDamage = 0;
	if(MaxDamage < 0)
		MaxDamage = 0;

	MinDefense = ArmorMinDefense + MinDefenseBonus;
	MaxDefense = ArmorMaxDefense + MaxDefenseBonus;
	if(MinDefense < 0)
		MinDefense = 0;
	if(MaxDefense < 0)
		MaxDefense = 0;
}

// Serialize action
void _Action::Serialize(_Buffer &Data) {

	uint32_t SkillID = 0;
	if(Skill)
		SkillID = Skill->ID;

	uint32_t ItemID = 0;
	if(Item)
		ItemID = Item->ID;

	Data.Write<uint32_t>(SkillID);
	Data.Write<uint32_t>(ItemID);
}

// Unserialize action
void _Action::Unserialize(_Buffer &Data, _Stats *Stats) {

	uint32_t SkillID = Data.Read<uint32_t>();
	uint32_t ItemID = Data.Read<uint32_t>();

	Skill = Stats->Skills[SkillID];
	Item = Stats->Items[ItemID];
}
