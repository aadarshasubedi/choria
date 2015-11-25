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
#include <network/oldnetwork.h>
#include <stdexcept>

// Constructor
_OldNetwork::_OldNetwork() {
}

// Destructor
_OldNetwork::~_OldNetwork() {
}

// Initializes enet
void _OldNetwork::InitializeSystem() {

	if(enet_initialize() != 0)
		throw std::runtime_error("enet_initialize() error");
}

// Closes enet
void _OldNetwork::CloseSystem() {
	enet_deinitialize();
}
