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
#include <framework.h>
#include <state.h>
#include <graphics.h>
#include <input.h>
#include <globals.h>
#include <random.h>
#include <objectmanager.h>
#include <config.h>
#include <stats.h>
#include <assets.h>
#include <constants.h>
#include <actions.h>
#include <menu.h>
#include <network/singlenetwork.h>
#include <network/multinetwork.h>
#include <states/editor.h>
#include <states/client.h>
#include <states/server.h>
#include <states/null.h>
#include <framelimit.h>
#include <SDL.h>
#include <string>

_Framework Framework;

// Processes parameters and initializes the game
void _Framework::Init(int ArgumentCount, char **Arguments) {
	FrameLimit = nullptr;
	LocalServerRunning = false;
	State = &NullState;
	TimeStepAccumulator = 0.0;
	TimeStep = GAME_TIMESTEP;
	RequestedState = nullptr;
	FrameworkState = INIT;
	Done = false;

	// Settings
	uint16_t NetworkPort = Config.NetworkPort;
	bool IsServer = false;

	// Process arguments
	std::string Token;
	int TokensRemaining;
	for(int i = 1; i < ArgumentCount; i++) {
		Token = std::string(Arguments[i]);
		TokensRemaining = ArgumentCount - i - 1;
		if(Token == "-host") {
			State = &ServerState;
			IsServer = true;
		}
		else if(Token == "-editor") {
			State = &EditorState;
			if(TokensRemaining && Arguments[i+1][0] != '-')
				EditorState.SetFilename(Arguments[++i]);
		}
		else if(Token == "-connect") {
			NullState.StartupMode = _NullState::CONNECT;
		}
		else if(Token == "-username" && TokensRemaining > 0) {
			Menu.SetUsername(Arguments[++i]);
		}
		else if(Token == "-password" && TokensRemaining > 0) {
			Menu.SetPassword(Arguments[++i]);
		}
		else if(Token == "-port" && TokensRemaining > 0) {
			NetworkPort = atoi(Arguments[++i]);
		}
		else if(Token == "-test") {
			ClientState.SetIsTesting(true);
		}
	}

	// Initialize network subsystem
	_Network::InitializeSystem();

	// Set up networking
	MultiNetwork = new _MultiNetwork();
	ClientSingleNetwork = new _SingleNetwork();
	ServerSingleNetwork = new _SingleNetwork();
	MultiNetwork->Init(IsServer, NetworkPort);
	ClientSingleNetwork->Init(false);
	ServerSingleNetwork->Init(false);
	if(IsServer)
		ServerNetwork = MultiNetwork;
	else
		ClientNetwork = MultiNetwork;

	// Set random seed
	RandomGenerator.seed(SDL_GetPerformanceCounter());

	// Check state
	if(State == &ServerState) {

		// Open log
		Log.Open((Config.ConfigPath + "server.log").c_str());

		Assets.Init(true);
		Stats.Init();
		ServerState.StartCommandThread();
		FrameLimit = new _FrameLimit(120.0, false);
	}
	else {

		// Open log
		Log.Open((Config.ConfigPath + "client.log").c_str());

		// Initialize SDL
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
			throw std::runtime_error("Failed to initialize SDL");

		// Get fullscreen size
		Config.SetDefaultFullscreenSize();

		// Initialize graphics system
		_WindowSettings WindowSettings;
		WindowSettings.WindowTitle = "choria";
		WindowSettings.Fullscreen = Config.Fullscreen;
		if(Config.Fullscreen)
			WindowSettings.Size = Config.FullscreenSize;
		else
			WindowSettings.Size = Config.WindowSize;
		WindowSettings.Position = glm::ivec2(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		Graphics.Init(WindowSettings);
		Assets.Init(false);
		Graphics.SetStaticUniforms();

		// Set up the stats system
		Stats.Init();

		FrameLimit = new _FrameLimit(120.0, false);
	}

	Timer = SDL_GetPerformanceCounter();
}

// Shuts down the game
void _Framework::Close() {

	// Close the state
	State->Close();
	if(LocalServerRunning)
		ServerState.Close();

	if(MultiNetwork)
		MultiNetwork->WaitForDisconnect();
	Config.Save();

	// Shut down the system
	_Network::CloseSystem();
	if(MultiNetwork)
		MultiNetwork->Close();
	if(ClientSingleNetwork)
		ClientSingleNetwork->Close();
	if(ServerSingleNetwork)
		ServerSingleNetwork->Close();
	delete MultiNetwork;
	delete ClientSingleNetwork;
	delete ServerSingleNetwork;
	Assets.Close();
	Stats.Close();
	Graphics.Close();
	Config.Close();

	delete FrameLimit;
	if(SDL_WasInit(SDL_INIT_VIDEO))
		SDL_Quit();
}

// Requests a state change
void _Framework::ChangeState(_State *RequestedState) {
	this->RequestedState = RequestedState;
	FrameworkState = CLOSE;
}

// Updates the current state and manages the state stack
void _Framework::Update() {
	double FrameTime = (SDL_GetPerformanceCounter() - Timer) / (double)SDL_GetPerformanceFrequency();
	Timer = SDL_GetPerformanceCounter();

	SDL_PumpEvents();
	Input.Update(FrameTime);

	// Loop through events
	SDL_Event Event;
	while(SDL_PollEvent(&Event)) {
		switch(Event.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				if(!Event.key.repeat) {

					// Toggle fullscreen
					if(Event.type == SDL_KEYDOWN && (Event.key.keysym.mod & KMOD_ALT) && Event.key.keysym.scancode == SDL_SCANCODE_RETURN)
						Graphics.ToggleFullScreen(Config.WindowSize, Config.FullscreenSize);
					else if(State && FrameworkState == UPDATE) {
						_KeyEvent KeyEvent(Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
						State->KeyEvent(KeyEvent);
						Actions.InputEvent(_Input::KEYBOARD, Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
					}
				}
				else {
					if(State && FrameworkState == UPDATE && Event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE) {
						_KeyEvent KeyEvent(Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
						State->KeyEvent(KeyEvent);
					}
				}
			break;
			case SDL_TEXTINPUT:
				if(State)
					State->TextEvent(Event.text.text);
			break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				if(State && FrameworkState == UPDATE) {
					_MouseEvent MouseEvent(glm::ivec2(Event.motion.x, Event.motion.y), Event.button.button, Event.type == SDL_MOUSEBUTTONDOWN);
					State->MouseEvent(MouseEvent);
					Actions.InputEvent(_Input::MOUSE_BUTTON, Event.button.button, Event.type == SDL_MOUSEBUTTONDOWN);
				}
			break;
			case SDL_MOUSEWHEEL:
				if(State)
					State->MouseWheelEvent(Event.wheel.y);
			break;
			case SDL_WINDOWEVENT:
				if(Event.window.event)
					State->WindowEvent(Event.window.event);
			break;
			case SDL_QUIT:
				Done = true;
			break;
		}
	}

	switch(FrameworkState) {
		case INIT: {
			if(State) {
				State->Init();
				FrameworkState = UPDATE;
			}
			else
				Done = true;
		} break;
		case UPDATE: {
			TimeStepAccumulator += FrameTime;
			while(TimeStepAccumulator >= TimeStep) {
				if(LocalServerRunning)
					ServerState.Update(TimeStep);
				else
					MultiNetwork->Update();

				State->Update(TimeStep);
				TimeStepAccumulator -= TimeStep;
			}

			State->Render(TimeStepAccumulator);
		} break;
		case CLOSE: {
			if(State)
				State->Close();

			State = RequestedState;
			FrameworkState = INIT;
		} break;
		default:
		break;
	}

	Graphics.Flip(FrameTime);

	if(FrameLimit)
		FrameLimit->Update();
}

// Starts the local server
void _Framework::StartLocalServer() {
	if(!LocalServerRunning) {
		LocalServerRunning = true;

		ClientNetwork = ClientSingleNetwork;
		ServerNetwork = ServerSingleNetwork;

		ServerState.Init();
	}
}

// Stops the local server
void _Framework::StopLocalServer() {
	if(LocalServerRunning) {
		ClientNetwork->Disconnect();
		LocalServerRunning = false;

		ServerState.Close();

		ClientNetwork = MultiNetwork;
		ServerNetwork = MultiNetwork;
	}
}
