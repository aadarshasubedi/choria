/*************************************************************************************
*	Choria - http://choria.googlecode.com/
*	Copyright (C) 2010  Alan Witkowski
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
**************************************************************************************/
#ifndef CREATECHARACTER_H
#define CREATECHARACTER_H

// Libraries
#include "engine/state.h"

// Forward Declarations
class PacketClass;

// Classes
class CreateCharacterState : public StateClass {

	public:

		enum StateType {
			STATE_MAIN,
			STATE_SEND,
		};

		enum ElementType {
			ELEMENT_NAME,
			ELEMENT_CREATE,
			ELEMENT_BACK,
			ELEMENT_PORTRAITS,
		};

		int Init();
		int Close();
		
		void HandleDisconnect(ENetEvent *TEvent);
		void HandlePacket(ENetEvent *TEvent);
		bool HandleKeyPress(EKEY_CODE TKey);
		void HandleGUI(EGUI_EVENT_TYPE TEventType, IGUIElement *TElement);

		void Update(u32 TDeltaTime);
		void Draw();

		static CreateCharacterState *Instance() {
			static CreateCharacterState ClassInstance;
			return &ClassInstance;
		}

	private:

		void UpdateSelection(int TSelectedIndex);
		void CreateCharacter();
		void Back();
		
		// States
		int State;

		// GUI
		IGUIEditBox *EditName;
		IGUIButton *ButtonCreate, *ButtonBack, *SelectedButton;
		stringc Message;
		int SelectedIndex;

		array<int> Portraits;		
};

#endif