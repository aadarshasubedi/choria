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
#include <menu.h>
#include <constants.h>
#include <input.h>
#include <actions.h>
#include <graphics.h>
#include <assets.h>
#include <config.h>
#include <framework.h>
#include <buffer.h>
#include <stats.h>
#include <ui/element.h>
#include <ui/label.h>
#include <ui/button.h>
#include <ui/image.h>
#include <ui/style.h>
#include <ui/textbox.h>
#include <network/network.h>
#include <states/null.h>
#include <states/playclient.h>
#include <sstream>
#include <SDL_mouse.h>

#include <globals.h>

_Menu Menu;

const std::string InputBoxPrefix = "button_options_input_";
const std::string CharacterButtonPrefix = "button_characters_slot";
const std::string CharacterNamePrefix = "label_menu_characters_slot_name";
const std::string CharacterLevelPrefix = "label_menu_characters_slot_level";
const std::string CharacterPortraitPrefix = "button_newcharacter_portrait";

// Constructor
_Menu::_Menu() {
	State = STATE_NONE;
	CurrentLayout = nullptr;
	Background = nullptr;
	OptionsState = OPTION_NONE;
	CharactersState = CHARACTERS_NONE;
	PreviousClickTimer = 0.0;
}

// Initialize
void _Menu::InitTitle() {
	Assets.Labels["label_menu_title_version"]->Text = GAME_VERSION;

	CurrentLayout = Assets.Elements["element_menu_title"];
	Framework.StopLocalServer();

	State = STATE_TITLE;
}

// Init single player
void _Menu::InitCharacters() {
	CurrentLayout = Assets.Elements["element_menu_characters"];
	CharactersState = CHARACTERS_NONE;
	State = STATE_CHARACTERS;
}

// Options
void _Menu::InitOptions() {
	//CurrentLayout = Assets.Elements["element_menu_options"];

	RefreshInputLabels();
	CurrentAction = -1;

	OptionsState = OPTION_NONE;
	State = STATE_OPTIONS;
}

// In-game menu
void _Menu::InitInGame() {
	//CurrentLayout = Assets.Elements["element_menu_ingame"];

	Background = nullptr;

	State = STATE_INGAME;
}

// Return to play
void _Menu::InitPlay() {
	CurrentLayout = nullptr;

	State = STATE_NONE;
}

// Init new player popup
void _Menu::InitNewCharacter() {
	_Button *CreateButton = Assets.Buttons["button_newcharacter_create"];
	CreateButton->Enabled = false;

	_TextBox *Name = Assets.TextBoxes["textbox_newcharacter_name"];
	Name->Focused = true;
	Name->Text = "";
	Name->ResetCursor();

	LoadPortraitButtons();

	CurrentLayout = Assets.Elements["element_menu_new"];
	CharactersState = CHARACTERS_CREATE;
}

// Play the game
void _Menu::LaunchGame() {
	//Save.GetPlayer(SelectedSlot)->Load();
	//PlayState.SetPlayer(Save.GetPlayer(SelectedSlot));
	//ClientState.SetLevel("");
	//ClientState.SetTestMode(false);
	//ClientState.SetFromEditor(false);
	//Framework.ChangeState(&ClientState);

	//SaveSlots[SelectedSlot]->Enabled = false;
	//State = STATE_NONE;
}

// Get the selected portrait index
int _Menu::GetSelectedPortrait() {
	int Index = 0;

	// Check for selected portrait
	_Element *PortraitsElement = Assets.Elements["element_menu_new_portraits"];
	for(auto &Element : PortraitsElement->Children) {
		_Button *Button = (_Button *)Element;
		if(Button->Checked)
			return Index;

		Index++;
	}

	return -1;
}

// Get the selected character slot
int _Menu::GetSelectedCharacter() {
	int Index = 0;

	// Check for selected character
	_Element *CharactersElement = Assets.Elements["element_menu_characters"];
	for(auto &Element : CharactersElement->Children) {
		if(Element->Identifier.substr(0, CharacterButtonPrefix.size()) == CharacterButtonPrefix) {
			_Button *Button = (_Button *)Element;
			if(Button->Checked)
				return Index;

			Index++;
		}
	}

	return -1;
}

// Create character
void _Menu::CreateCharacter() {
	_TextBox *Name = Assets.TextBoxes["textbox_newcharacter_name"];
	if(Name->Text.length() == 0)
		return;

	int SelectedPortrait = GetSelectedPortrait();
	if(SelectedPortrait == -1)
		return;

	// Send information
	_Buffer Packet;
	Packet.Write<char>(_Network::CREATECHARACTER_INFO);
	Packet.WriteString(Name->Text.c_str());
	Packet.Write<int32_t>(SelectedPortrait);
	ClientNetwork->SendPacketToHost(&Packet);

	// Close new character screen
	RequestCharacterList();
}

// Request character list from server
void _Menu::RequestCharacterList() {

	// Request character list
	_Buffer Packet;
	Packet.Write<char>(_Network::CHARACTERS_REQUEST);
	ClientNetwork->SendPacketToHost(&Packet);
}

// Load portraits
void _Menu::LoadPortraitButtons() {

	// Clear old children
	_Element *PortraitsElement = Assets.Elements["element_menu_new_portraits"];
	std::vector<_Element *> &Children = PortraitsElement->Children;
	for(size_t i = 0; i < Children.size(); i++) {
		delete Children[i]->Style;
		delete Children[i];
	}
	Children.clear();

	glm::ivec2 Offset(10, 0);
	int Width = PortraitsElement->Size.x;
	size_t i = 0;

	// Iterate over portraits
	std::list<_Portrait> Portraits;
	Stats.GetPortraits(Portraits);
	for(auto &Portrait : Portraits) {

		// Create style
		_Style *Style = new _Style;
		Style->TextureColor = COLOR_WHITE;
		Style->Program = Assets.Programs["ortho_pos_uv"];
		Style->Texture = Portrait.Image;

		// Add button
		_Button *Button = new _Button();
		Button->Identifier = CharacterPortraitPrefix;
		Button->Parent = PortraitsElement;
		Button->Offset = Offset;
		Button->Size = Portrait.Image->Size;
		Button->Alignment = LEFT_TOP;
		Button->Style = Style;
		Button->HoverStyle = Assets.Styles["style_menu_portrait_hover"];
		Button->UserData = (void *)i;
		PortraitsElement->Children.push_back(Button);

		// Update position
		Offset.x += Portrait.Image->Size.x + 10;
		if(Offset.x > Width - Portrait.Image->Size.x + 10) {
			Offset.y += Portrait.Image->Size.y + 10;
			Offset.x = 10;
		}

		i++;
	}

	PortraitsElement->CalculateBounds();
}

// Check new character screen for portrait and name
void _Menu::ValidateCreateCharacter() {
	bool NameValid = false;

	int PortraitIndex = GetSelectedPortrait();

	// Check name length
	_Button *CreateButton = Assets.Buttons["button_newcharacter_create"];
	_TextBox *Name = Assets.TextBoxes["textbox_newcharacter_name"];
	if(Name->Text.length() > 0)
		NameValid = true;

	// Enable button
	if(PortraitIndex != -1 && NameValid)
		CreateButton->Enabled = true;
	else
		CreateButton->Enabled = false;
}

// Validate characters ui elements
void _Menu::UpdateCharacterButtons() {
	_Button *DeleteButton = Assets.Buttons["button_characters_delete"];
	_Button *PlayButton = Assets.Buttons["button_characters_play"];
	DeleteButton->Enabled = false;
	PlayButton->Enabled = false;

	int SelectedSlot = GetSelectedCharacter();
	if(SelectedSlot != -1 && CharacterSlots[SelectedSlot].Used) {
		DeleteButton->Enabled = true;
		PlayButton->Enabled = true;
	}
}

// Shutdown
void _Menu::Close() {
}

// Handle key event
void _Menu::KeyEvent(const _KeyEvent &KeyEvent) {
	if(CurrentLayout)
		CurrentLayout->HandleKeyEvent(KeyEvent);

	switch(State) {
		case STATE_TITLE: {
			if(KeyEvent.Pressed && KeyEvent.Key == SDL_SCANCODE_ESCAPE)
				Framework.SetDone(true);
		} break;
		case STATE_CHARACTERS: {

			if(CharactersState == CHARACTERS_NONE) {
				if(KeyEvent.Pressed && KeyEvent.Key == SDL_SCANCODE_ESCAPE)
					InitTitle();
			}
			else {
				if(KeyEvent.Pressed) {
					ValidateCreateCharacter();

					if(KeyEvent.Key == SDL_SCANCODE_ESCAPE)
						RequestCharacterList();
					else if(KeyEvent.Key == SDL_SCANCODE_RETURN)
						CreateCharacter();
				}
			}
		} break;
		case STATE_OPTIONS: {
			if(OptionsState == OPTION_NONE) {
				if(KeyEvent.Pressed && KeyEvent.Key == SDL_SCANCODE_ESCAPE) {
					//Config.Save();
					//if(Framework.GetState() == &ClientState)
					//	InitInGame();
					//else
					//	InitTitle();
				}
			}
			else {
				if(KeyEvent.Pressed) {
					RemapInput(_Input::KEYBOARD, KeyEvent.Key);
				}
			}
		} break;
		case STATE_INGAME: {
			if(KeyEvent.Pressed && KeyEvent.Key == SDL_SCANCODE_ESCAPE)
				InitPlay();
		} break;
		default:
		break;
	}
}

// Handle text
void _Menu::TextEvent(const char *Text) {
	if(CurrentLayout)
		CurrentLayout->HandleTextEvent(Text);

	switch(State) {
		case STATE_CHARACTERS:
			if(CharactersState == CHARACTERS_CREATE) {
				ValidateCreateCharacter();
			}
		break;
		default:
		break;
	}
}

// Handle mouse event
void _Menu::MouseEvent(const _MouseEvent &MouseEvent) {
	if(!CurrentLayout)
		return;

	// Accepting new action input
	switch(State) {
		case STATE_OPTIONS: {
			if(OptionsState == OPTION_ACCEPT_INPUT) {
				if(MouseEvent.Pressed) {
					RemapInput(_Input::MOUSE_BUTTON, MouseEvent.Button);
					return;
				}
			}
		} break;
		default:
		break;
	}

	if(MouseEvent.Button == SDL_BUTTON_LEFT)
		CurrentLayout->HandleInput(MouseEvent.Pressed);

	// Get clicked element
	_Element *Clicked = CurrentLayout->GetClickedElement();
	if(Clicked) {
		bool DoubleClick = false;
		if(PreviousClick == Clicked && PreviousClickTimer < MENU_DOUBLECLICK_TIME) {
			PreviousClick = nullptr;
			DoubleClick = true;
		}
		else
			PreviousClick = Clicked;
		PreviousClickTimer = 0.0;

		switch(State) {
			case STATE_TITLE: {
				if(Clicked->Identifier == "button_title_singleplayer") {
					Connect("", true);
				}
				else if(Clicked->Identifier == "button_title_multiplayer") {
				}
				else if(Clicked->Identifier == "button_title_mapeditor") {
				}
				else if(Clicked->Identifier == "button_title_exit") {
					Framework.SetDone(true);
				}
			} break;
			case STATE_CHARACTERS: {
				if(CharactersState == CHARACTERS_NONE) {

					if(Clicked->Identifier == "button_characters_delete") {
						int SelectedSlot = GetSelectedCharacter();
						if(SelectedSlot != -1 && CharacterSlots[SelectedSlot].Used) {
							_Buffer Packet;
							Packet.Write<char>(_Network::CHARACTERS_DELETE);
							Packet.Write<char>(SelectedSlot);
							ClientNetwork->SendPacketToHost(&Packet);
						}
					}
					else if(Clicked->Identifier == "button_characters_play") {
						int SelectedSlot = GetSelectedCharacter();
						if(SelectedSlot != -1 && CharacterSlots[SelectedSlot].Used) {
							PlayClientState.SetCharacterSlot(SelectedSlot);
							Framework.ChangeState(&PlayClientState);
						}
					}
					else if(Clicked->Identifier == "button_characters_back") {
						InitTitle();
					}
					else if(Clicked->Identifier.substr(0, CharacterButtonPrefix.size()) == CharacterButtonPrefix) {

						// Deselect slots
						_Element *CharactersElement = Assets.Elements["element_menu_characters"];
						for(auto &Element : CharactersElement->Children) {
							if(Element->Identifier.substr(0, CharacterButtonPrefix.size()) == CharacterButtonPrefix) {
								_Button *Button = (_Button *)Element;
								Button->Checked = false;
							}
						}

						int SelectedSlot = (intptr_t)Clicked->UserData;
						if(CharacterSlots[SelectedSlot].Used) {
							CharacterSlots[SelectedSlot].Button->Checked = true;
						}
						else
							InitNewCharacter();

						UpdateCharacterButtons();

						if(DoubleClick) {
							LaunchGame();
						}
					}
				}
				else if(CharactersState == CHARACTERS_CREATE) {
					if(Clicked->Identifier == CharacterPortraitPrefix) {
						int Selected = (intptr_t)Clicked->UserData;

						// Unselect all portraits and select the clicked element
						int i = 0;
						for(auto &Element : Clicked->Parent->Children) {
							_Button *Button = (_Button *)Element;
							Button->Checked = false;
							if(i == Selected) {
								_TextBox *Name = Assets.TextBoxes["textbox_newcharacter_name"];
								Name->Focused = true;
								Name->ResetCursor();
								Button->Checked = true;
							}

							i++;
						}

						ValidateCreateCharacter();
					}
					else if(Clicked->Identifier == "button_newcharacter_create") {
						CreateCharacter();
					}
					else if(Clicked->Identifier == "button_newcharacter_cancel") {
						RequestCharacterList();
					}
				}
			} break;
			case STATE_OPTIONS: {
				if(OptionsState == OPTION_NONE) {
					if(Clicked->Identifier == "button_options_defaults") {
						Config.LoadDefaultInputBindings();
						RefreshInputLabels();
					}
					else if(Clicked->Identifier == "button_options_save") {
						Config.Save();
						//if(Framework.GetState() == &ClientState)
						//	InitInGame();
						//else
						//	InitTitle();
					}
					else if(Clicked->Identifier == "button_options_cancel") {
						Config.Load();
						//if(Framework.GetState() == &ClientState)
						//	InitInGame();
						//else
						//	InitTitle();
					}
					else if(Clicked->Identifier.substr(0, InputBoxPrefix.size()) == InputBoxPrefix) {
						OptionsState = OPTION_ACCEPT_INPUT;
						CurrentAction = (intptr_t)Clicked->UserData;
						Assets.Labels["label_menu_options_accept_action"]->Text = Actions.GetName(CurrentAction);
					}
				}
			} break;
			case STATE_INGAME: {
				if(Clicked->Identifier == "button_ingame_resume") {
					InitPlay();
				}
				else if(Clicked->Identifier == "button_ingame_options") {
					InitOptions();
				}
				else if(Clicked->Identifier == "button_ingame_menu") {
					Framework.ChangeState(&NullState);
				}
			} break;
			default:
			break;
		}
	}
}

// Update phase
void _Menu::Update(double FrameTime) {
	PreviousClickTimer += FrameTime;

	if(CurrentLayout && OptionsState == OPTION_NONE) {
		CurrentLayout->Update(FrameTime, Input.GetMouse());
	}

	switch(State) {
		case STATE_TITLE: {
		} break;
		case STATE_CHARACTERS: {
		} break;
		case STATE_OPTIONS: {
		} break;
		default:
		break;
	}
}

// Draw phase
void _Menu::Render() {
	Graphics.Setup2D();

	if(Background)
		Background->Render();

	switch(State) {
		case STATE_TITLE: {
			if(CurrentLayout)
				CurrentLayout->Render();
			Assets.Labels["label_menu_title_version"]->Render();
		} break;
		case STATE_OPTIONS: {
			if(CurrentLayout)
				CurrentLayout->Render();

			if(OptionsState == OPTION_ACCEPT_INPUT) {
				Graphics.FadeScreen(MENU_ACCEPTINPUT_FADE);
				Assets.Elements["element_menu_popup"]->Render();
			}
		} break;
		case STATE_CHARACTERS: {
			Assets.Elements["element_menu_characters"]->Render();

			if(CharactersState == CHARACTERS_CREATE) {
				Graphics.FadeScreen(MENU_ACCEPTINPUT_FADE);
				if(CurrentLayout)
					CurrentLayout->Render();
			}

		} break;
		case STATE_INGAME: {
			if(CurrentLayout)
				CurrentLayout->Render();
		} break;
		default:
		break;
	}
}

void _Menu::HandleConnect(ENetEvent *TEvent) {
}

void _Menu::HandleDisconnect(ENetEvent *TEvent) {

}

// Handle packet
void _Menu::HandlePacket(ENetEvent *TEvent) {
	_Buffer Packet((char *)TEvent->packet->data, TEvent->packet->dataLength);
	switch(Packet.Read<char>()) {
		case _Network::VERSION: {
			std::string Version(Packet.ReadString());
			Framework.Log << "_Network::VERSION=" << Version << std::endl;
			if(Version != GAME_VERSION) {
				//Message = "Game version differs from server's";
				//ChangeState(STATE_MAIN);
			}
		}
		break;
		case _Network::ACCOUNT_SUCCESS: {
			Framework.Log << "_Network::ACCOUNT_SUCCESS" << std::endl;
			RequestCharacterList();
		}
		break;
		case _Network::CHARACTERS_LIST: {
			Framework.Log << "_Network::CHARACTERS_LIST" << std::endl;

			// Get count
			int CharacterCount = Packet.Read<char>();
			Framework.Log << "CharacterCount=" << CharacterCount << std::endl;

			// Set up character slots
			for(int i = 0; i < SAVE_COUNT; i++) {
				std::stringstream Buffer;

				// Set slot name
				Buffer << CharacterNamePrefix << i;
				CharacterSlots[i].Name = Assets.Labels[Buffer.str()];
				if(!CharacterSlots[i].Name)
					throw std::runtime_error("Can't find label: " + Buffer.str());
				Buffer.str("");

				// Set slot level
				Buffer << CharacterLevelPrefix << i;
				CharacterSlots[i].Level = Assets.Labels[Buffer.str()];
				if(!CharacterSlots[i].Level)
					throw std::runtime_error("Can't find label: " + Buffer.str());
				Buffer.str("");

				// Assign button
				Buffer << CharacterButtonPrefix << i;
				CharacterSlots[i].Button = Assets.Buttons[Buffer.str()];
				CharacterSlots[i].Button->Checked = false;

				// Set state
				CharacterSlots[i].Name->Text = "Empty Slot";
				CharacterSlots[i].Level->Text = "";
				CharacterSlots[i].Used = false;
			}

			//if(CharacterCount < SAVE_COUNT)
			//	ButtonCreate->setEnabled(true);

			// Get characters
			for(int i = 0; i < CharacterCount; i++) {
				CharacterSlots[i].Name->Text = Packet.ReadString();
				int32_t PortraitIndex = Packet.Read<int32_t>();
				int32_t Experience = Packet.Read<int32_t>();

				std::stringstream Buffer;
				Buffer << "Level " << Stats.FindLevel(Experience)->Level;
				CharacterSlots[i].Level->Text = Buffer.str();
				CharacterSlots[i].Used = true;
				//CharacterSlots[i].Button->Style->Texture =
				//Slots[i].Used = true;
				//Slots[i].Name = TPacket->ReadString();
				//const _Texture *PortraitImage = Stats.GetPortrait(PortraitIndex)->Image;
				//CharacterSlots[i].Button->Style->Texture = PortraitImage;
				//Slots[i].Button->setImage(PortraitImage);
				//Slots[i].Button->setPressedImage(PortraitImage);
				//Slots[i].Level = Stats.FindLevel(TPacket->Read<int32_t>())->Level;
			}

			UpdateCharacterButtons();

			InitCharacters();
		}
		break;
		case _Network::CREATECHARACTER_SUCCESS:
			//Framework.ChangeState(&CharactersState);
		break;
		case _Network::CREATECHARACTER_INUSE:
			//Message = "Character name already in use";
			//ButtonCreate->setEnabled(true);
		break;
	}
}

// Connect to a server
void _Menu::Connect(const std::string &Address, bool Fake) {

	// Connect to the fake singleplayer network
	if(Fake) {
		Framework.StartLocalServer();
		ClientNetwork->Connect("");

		// Send fake account information
		{
			_Buffer Packet;
			Packet.Write<char>(_Network::ACCOUNT_LOGININFO);
			Packet.WriteBit(0);
			Packet.WriteString("singleplayer");
			Packet.WriteString("singleplayer");
			ClientNetwork->SendPacketToHost(&Packet);
		}
	}
}

// Refreshes the input map labels
void _Menu::RefreshInputLabels() {
	/*for(size_t i = 0; i < LABEL_COUNT; i++) {
		InputLabels[i] = Assets.Labels[KEYLABEL_IDENTIFIERS[i]];
		InputLabels[i]->Text = Actions.GetInputNameForAction(i);
		InputLabels[i]->Parent->UserData = (void *)(intptr_t)i;
	}*/
}

// Remap a key/button
void _Menu::RemapInput(int InputType, int Input) {
	/*OptionsState = OPTION_NONE;
	if(InputType == _Input::KEYBOARD && Input == SDL_SCANCODE_ESCAPE)
		return;

	// Remove duplicate keys/buttons
	for(int i = 0; i < _Actions::COUNT; i++) {
		if(Actions.GetInputForAction(InputType, i) == Input) {
			Actions.ClearMappingsForAction(InputType, i);
		}
	}

	// Clear out existing action
	Actions.ClearMappingsForAction(_Input::KEYBOARD, CurrentAction);
	Actions.ClearMappingsForAction(_Input::MOUSE_BUTTON, CurrentAction);

	// Add new binding
	Actions.AddInputMap(InputType, Input, CurrentAction, false);

	// Update menu labels
	RefreshInputLabels();
	*/
}
