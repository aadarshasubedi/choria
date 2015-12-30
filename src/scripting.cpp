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
#include <scripting.h>
#include <objects/object.h>
#include <objects/buff.h>
#include <objects/statchange.h>
#include <objects/statuseffect.h>
#include <objects/battle.h>
#include <objects/inventory.h>
#include <stats.h>
#include <random.h>
#include <stdexcept>
#include <iostream>

// Controls
luaL_Reg _Scripting::RandomFunctions[] = {
	{"GetInt", &_Scripting::RandomGetInt},
	{nullptr, nullptr}
};

// Lua library functions
int luaopen_Object(lua_State *LuaState) {
	luaL_newlib(LuaState, _Scripting::RandomFunctions);
	return 1;
}

// Constructor
_Scripting::_Scripting() :
	LuaState(nullptr),
	CurrentTableIndex(0) {

	// Initialize lua object
	LuaState = luaL_newstate();
	luaL_openlibs(LuaState);
	luaL_requiref(LuaState, "Random", luaopen_Object, 1);
}

// Destructor
_Scripting::~_Scripting() {

	// Close lua state
	if(LuaState != nullptr)
		lua_close(LuaState);
}

// Load a script file
void _Scripting::LoadScript(const std::string &Path) {

	// Load the file
	if(luaL_dofile(LuaState, Path.c_str()))
		throw std::runtime_error("Failed to load script " + Path + "\n" + std::string(lua_tostring(LuaState, -1)));
}

// Load global state with stat info
void _Scripting::InjectStats(_Stats *Stats) {
	lua_newtable(LuaState);

	// Iterate through buffs
	for(const auto &Iterator : Stats->Buffs) {
		const _Buff *Buff = Iterator.second;
		if(Buff) {

			// Add pointer to table
			lua_pushstring(LuaState, Buff->Script.c_str());
			lua_pushlightuserdata(LuaState, (void *)Buff);
			lua_settable(LuaState, -3);
		}
	}

	// Assign table a name
	lua_setglobal(LuaState, "Buffs");

	// Push inventory slot types
	lua_pushinteger(LuaState, InventoryType::HEAD);
	lua_setglobal(LuaState, "INVENTORY_HEAD");
	lua_pushinteger(LuaState, InventoryType::BODY);
	lua_setglobal(LuaState, "INVENTORY_BODY");
	lua_pushinteger(LuaState, InventoryType::LEGS);
	lua_setglobal(LuaState, "INVENTORY_LEGS");
	lua_pushinteger(LuaState, InventoryType::HAND1);
	lua_setglobal(LuaState, "INVENTORY_HAND1");
	lua_pushinteger(LuaState, InventoryType::HAND2);
	lua_setglobal(LuaState, "INVENTORY_HAND2");
	lua_pushinteger(LuaState, InventoryType::RING1);
	lua_setglobal(LuaState, "INVENTORY_RING1");
	lua_pushinteger(LuaState, InventoryType::RING2);
	lua_setglobal(LuaState, "INVENTORY_RING2");
}

// Push object onto stack
void _Scripting::PushObject(_Object *Object) {
	lua_newtable(LuaState);

	lua_pushlightuserdata(LuaState, Object);
	lua_pushcclosure(LuaState, &ObjectSetBattleTarget, 1);
	lua_setfield(LuaState, -2, "SetBattleTarget");

	lua_pushlightuserdata(LuaState, Object);
	lua_pushcclosure(LuaState, &ObjectGetInventoryItem, 1);
	lua_setfield(LuaState, -2, "GetInventoryItem");

	lua_pushlightuserdata(LuaState, Object);
	lua_pushcclosure(LuaState, &ObjectSetAction, 1);
	lua_setfield(LuaState, -2, "SetAction");

	lua_pushlightuserdata(LuaState, Object);
	lua_pushcclosure(LuaState, &ObjectGenerateDamage, 1);
	lua_setfield(LuaState, -2, "GenerateDamage");

	lua_pushlightuserdata(LuaState, Object);
	lua_pushcclosure(LuaState, &ObjectGenerateDefense, 1);
	lua_setfield(LuaState, -2, "GenerateDefense");

	lua_pushnumber(LuaState, Object->TurnTimer);
	lua_setfield(LuaState, -2, "TurnTimer");

	lua_pushboolean(LuaState, Object->Action.IsSet());
	lua_setfield(LuaState, -2, "BattleActionIsSet");

	lua_pushlightuserdata(LuaState, Object->Targets.front());
	lua_setfield(LuaState, -2, "BattleTarget");

	lua_pushinteger(LuaState, Object->BattleSide);
	lua_setfield(LuaState, -2, "BattleSide");

	lua_pushnumber(LuaState, Object->Health);
	lua_setfield(LuaState, -2, "Health");

	lua_pushnumber(LuaState, Object->MaxHealth);
	lua_setfield(LuaState, -2, "MaxHealth");

	lua_pushnumber(LuaState, Object->Mana);
	lua_setfield(LuaState, -2, "Mana");

	lua_pushnumber(LuaState, Object->MaxMana);
	lua_setfield(LuaState, -2, "MaxMana");

	lua_pushnumber(LuaState, Object->HitChance);
	lua_setfield(LuaState, -2, "HitChance");

	lua_pushnumber(LuaState, Object->Evasion);
	lua_setfield(LuaState, -2, "Evasion");

	lua_pushlightuserdata(LuaState, Object);
	lua_setfield(LuaState, -2, "Pointer");
}

// Push item onto stack
void _Scripting::PushItem(lua_State *LuaState, const _Item *Item) {
	if(!Item) {
		lua_pushnil(LuaState);
		return;
	}

	lua_newtable(LuaState);

	lua_pushlightuserdata(LuaState, (void *)Item);
	lua_pushcclosure(LuaState, &ItemGenerateDefense, 1);
	lua_setfield(LuaState, -2, "GenerateDefense");

	lua_pushlightuserdata(LuaState, (void *)Item);
	lua_setfield(LuaState, -2, "Pointer");
}

// Push action result onto stack
void _Scripting::PushActionResult(_ActionResult *ActionResult) {
	lua_newtable(LuaState);

	PushStatChange(&ActionResult->Source);
	lua_setfield(LuaState, -2, "Source");

	PushStatChange(&ActionResult->Target);
	lua_setfield(LuaState, -2, "Target");
}

// Push stat change struct onto stack
void _Scripting::PushStatChange(_StatChange *StatChange) {
	lua_newtable(LuaState);

	lua_pushnumber(LuaState, StatChange->Health);
	lua_setfield(LuaState, -2, "Health");

	lua_pushnumber(LuaState, StatChange->MaxHealth);
	lua_setfield(LuaState, -2, "MaxHealth");

	lua_pushnumber(LuaState, StatChange->Mana);
	lua_setfield(LuaState, -2, "Mana");

	lua_pushnumber(LuaState, StatChange->MaxMana);
	lua_setfield(LuaState, -2, "MaxMana");

	lua_pushinteger(LuaState, StatChange->Invisible);
	lua_setfield(LuaState, -2, "Invisible");

	lua_pushinteger(LuaState, StatChange->ActionBarSize);
	lua_setfield(LuaState, -2, "ActionBarSize");
}

// Push list of objects
void _Scripting::PushObjectList(std::list<_Object *> &Objects) {
	lua_newtable(LuaState);

	int Index = 1;
	for(const auto &Object : Objects) {
		PushObject(Object);
		lua_rawseti(LuaState, -2, Index);

		Index++;
	}
}

// Push int value
void _Scripting::PushInt(int Value) {
	lua_pushinteger(LuaState, Value);
}

// Get return value as int
int _Scripting::GetInt(int Index) {

	return (int)lua_tointeger(LuaState, Index + CurrentTableIndex);
}

// Get return value as bool
int _Scripting::GetBoolean(int Index) {

	return lua_toboolean(LuaState, Index + CurrentTableIndex);
}

// Get return value as string
std::string _Scripting::GetString(int Index) {

	return lua_tostring(LuaState, Index + CurrentTableIndex);
}

// Get return value as action result, Index=-1 means top of stack, otherwise index of return value
void _Scripting::GetActionResult(int Index, _ActionResult &ActionResult) {
	if(Index != -1)
		Index += CurrentTableIndex;

	// Check return value
	if(!lua_istable(LuaState, Index))
		throw std::runtime_error("GetActionResult: Value is not a table!");

	lua_pushstring(LuaState, "Source");
	lua_gettable(LuaState, -2);
	GetStatChange(-1, ActionResult.Source);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "Target");
	lua_gettable(LuaState, -2);
	GetStatChange(-1, ActionResult.Target);
	lua_pop(LuaState, 1);

}

// Get return value as stat change
void _Scripting::GetStatChange(int Index, _StatChange &StatChange) {
	if(Index != -1)
		Index += CurrentTableIndex;

	// Check return value
	if(!lua_istable(LuaState, Index))
		throw std::runtime_error("GetStatChange: Value is not a table!");

	lua_pushstring(LuaState, "Buff");
	lua_gettable(LuaState, -2);
	StatChange.StatusEffect.Buff = (_Buff *)lua_touserdata(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "BuffLevel");
	lua_gettable(LuaState, -2);
	StatChange.StatusEffect.Level = (int)lua_tointeger(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "BuffDuration");
	lua_gettable(LuaState, -2);
	StatChange.StatusEffect.Duration = (int)lua_tointeger(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "Health");
	lua_gettable(LuaState, -2);
	StatChange.Health = (float)lua_tonumber(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "MaxHealth");
	lua_gettable(LuaState, -2);
	StatChange.MaxHealth = (float)lua_tonumber(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "Mana");
	lua_gettable(LuaState, -2);
	StatChange.Mana = (float)lua_tonumber(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "MaxMana");
	lua_gettable(LuaState, -2);
	StatChange.MaxMana = (float)lua_tonumber(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "BattleSpeed");
	lua_gettable(LuaState, -2);
	StatChange.BattleSpeed = (float)lua_tonumber(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "Invisible");
	lua_gettable(LuaState, -2);
	StatChange.Invisible = (int)lua_tointeger(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "ActionBarSize");
	lua_gettable(LuaState, -2);
	StatChange.ActionBarSize = (int)lua_tointeger(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "Evasion");
	lua_gettable(LuaState, -2);
	StatChange.Evasion = (float)lua_tonumber(LuaState, -1);
	lua_pop(LuaState, 1);

	lua_pushstring(LuaState, "Miss");
	lua_gettable(LuaState, -2);
	StatChange.Miss = lua_toboolean(LuaState, -1);
	lua_pop(LuaState, 1);
}

// Start a call to a lua class method, return table index
bool _Scripting::StartMethodCall(const std::string &TableName, const std::string &Function) {

	// Find table
	lua_getglobal(LuaState, TableName.c_str());
	if(!lua_istable(LuaState, -1)) {
		lua_pop(LuaState, 1);

		return false;
	}

	// Save table index
	CurrentTableIndex = lua_gettop(LuaState);

	// Get function
	lua_getfield(LuaState, CurrentTableIndex, Function.c_str());
	if(!lua_isfunction(LuaState, -1)) {
		lua_pop(LuaState, 1);
		FinishMethodCall();

		return false;
	}

	// Push self parameter
	lua_getglobal(LuaState, TableName.c_str());

	return true;
}

// Run the function started by StartMethodCall
void _Scripting::MethodCall(int ParameterCount, int ReturnCount) {

	// Call function
	if(lua_pcall(LuaState, ParameterCount+1, ReturnCount, 0)) {
		throw std::runtime_error(lua_tostring(LuaState, -1));
	}
}

// Restore state
void _Scripting::FinishMethodCall() {

	// Restore stack
	lua_settop(LuaState, CurrentTableIndex - 1);
}

// Random.GetInt(min, max)
int _Scripting::RandomGetInt(lua_State *LuaState) {
	int Min = (int)lua_tointeger(LuaState, 1);
	int Max = (int)lua_tointeger(LuaState, 2);

	lua_pushinteger(LuaState, GetRandomInt(Min, Max));

	return 1;
}

// Set battle target
int _Scripting::ObjectSetBattleTarget(lua_State *LuaState) {
	if(!lua_istable(LuaState, 1))
		throw std::runtime_error("ObjectSetBattleTarget: Target is not a table!");

	// Get self pointer
	_Object *Object = (_Object *)lua_touserdata(LuaState, lua_upvalueindex(1));

	// Get pointer of target table
	lua_pushstring(LuaState, "Pointer");
	lua_gettable(LuaState, -2);
	_Object *Target = (_Object *)lua_touserdata(LuaState, -1);
	lua_pop(LuaState, 1);

	Object->Targets.push_back(Target);

	return 0;
}

// Return an item from the object's inventory
int _Scripting::ObjectGetInventoryItem(lua_State *LuaState) {
	if(!lua_isinteger(LuaState, 1))
		throw std::runtime_error("ObjectGetInventoryItem: Slot is not an integer!");

	// Get self pointer
	_Object *Object = (_Object *)lua_touserdata(LuaState, lua_upvalueindex(1));

	const _Item *Item = nullptr;

	// Get item
	size_t Slot = (size_t)lua_tointeger(LuaState, 1);
	if(Slot < Object->Inventory->Slots.size())
		Item = Object->Inventory->Slots[Slot].Item;

	// Push item
	PushItem(LuaState, Item);

	return 1;
}

// Set battle action
int _Scripting::ObjectSetAction(lua_State *LuaState) {

	// Get self pointer
	_Object *Object = (_Object *)lua_touserdata(LuaState, lua_upvalueindex(1));

	// Set skill used
	size_t ActionBarIndex = (size_t)lua_tointeger(LuaState, 1);
	Object->GetActionFromSkillbar(Object->Action, ActionBarIndex);

	return 0;
}

// Generate damage
int _Scripting::ObjectGenerateDamage(lua_State *LuaState) {

	_Object *Object = (_Object *)lua_touserdata(LuaState, lua_upvalueindex(1));
	lua_pushinteger(LuaState, Object->GenerateDamage());

	return 1;
}

// Generate defense
int _Scripting::ObjectGenerateDefense(lua_State *LuaState) {

	_Object *Object = (_Object *)lua_touserdata(LuaState, lua_upvalueindex(1));
	lua_pushinteger(LuaState, Object->GenerateDefense());

	return 1;
}

// Generate a random defense value for an item
int _Scripting::ItemGenerateDefense(lua_State *LuaState) {

	// Get self pointer
	_Item *Item = (_Item *)lua_touserdata(LuaState, lua_upvalueindex(1));
	lua_pushinteger(LuaState, GetRandomInt(Item->MinDefense, Item->MaxDefense));

	return 1;
}

// Print lua stack
void _Scripting::PrintStack(lua_State *LuaState) {
	for(int i = lua_gettop(LuaState); i >= 0; i--) {
		if(lua_isnumber(LuaState, i))
			std::cout << i << ": number : " << lua_tonumber(LuaState, i) << std::endl;
		else if(lua_isstring(LuaState, i))
			std::cout << i << ": string : " << lua_tostring(LuaState, i) << std::endl;
		else if(lua_istable(LuaState, i))
			std::cout << i << ": table" << std::endl;
		else if(lua_iscfunction(LuaState, i))
			std::cout << i << ": cfunction" << std::endl;
		else if(lua_isfunction(LuaState, i))
			std::cout << i << ": function" << std::endl;
		else if(lua_isuserdata(LuaState, i))
			std::cout << i << ": userdata" << std::endl;
		else if(lua_isnil(LuaState, i))
			std::cout << i << ": nil" << std::endl;
		else if(lua_islightuserdata(LuaState, i))
			std::cout << i << ": light userdata" << std::endl;
		else if(lua_isboolean(LuaState, i))
			std::cout << i << ": boolean : " << lua_toboolean(LuaState, i) << std::endl;
	}

	std::cout << "-----------------" << std::endl;
}
