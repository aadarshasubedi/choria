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
#include <network/network.h>

class _SingleNetwork : public _Network {

	public:

		void Init(bool Server);
		void Close();

		// Connections
		int Connect(const char *IPAddress, uint16_t Port);
		void Disconnect(ENetPeer *Peer=0);

		// Packets
		void SendPacketToHost(_Buffer *Buffer, SendType Type=RELIABLE, uint8_t Channel=0);
		void SendPacketToPeer(_Buffer *Buffer, ENetPeer *Peer, SendType Type=RELIABLE, uint8_t Channel=0);

	private:

		ENetPeer DummyPeer;
		bool Server, Connected;
};
