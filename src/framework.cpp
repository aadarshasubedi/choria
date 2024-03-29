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
#include <framework.h>
#include <states/editor.h>
#include <states/play.h>
#include <states/dedicated.h>
#include <states/bots.h>
#include <network/network.h>
#include <graphics.h>
#include <input.h>
#include <random.h>
#include <config.h>
#include <assets.h>
#include <constants.h>
#include <actions.h>
#include <menu.h>
#include <framelimit.h>
#include <SDL.h>
#include <string>

_Framework Framework;

// Processes parameters and initializes the game
void _Framework::Init(int ArgumentCount, char **Arguments) {
	FrameLimit = nullptr;
	TimeStepAccumulator = 0.0;
	TimeStep = DEFAULT_TIMESTEP;
	RequestedState = nullptr;
	FrameworkState = INIT;
	Done = false;

	// Settings
	std::string HostAddress = Config.LastHost;
	uint16_t NetworkPort = Config.NetworkPort;
	State = &PlayState;

	// Process arguments
	std::string Token;
	int TokensRemaining;
	for(int i = 1; i < ArgumentCount; i++) {
		Token = std::string(Arguments[i]);
		TokensRemaining = ArgumentCount - i - 1;
		if(Token == "-server") {
			State = &DedicatedState;
		}
		else if(Token == "-editor") {
			State = &EditorState;
			if(TokensRemaining && Arguments[i+1][0] != '-')
				EditorState.SetFilePath(Arguments[++i]);
		}
		else if(Token == "-connect") {
			PlayState.ConnectNow = true;
		}
		else if(Token == "-bot") {
			State = &BotState;
		}
		else if(Token == "-username" && TokensRemaining > 0) {
			Menu.SetUsername(Arguments[++i]);
		}
		else if(Token == "-password" && TokensRemaining > 0) {
			Menu.SetPassword(Arguments[++i]);
		}
		else if(Token == "-host" && TokensRemaining > 0) {
			HostAddress = Arguments[++i];
		}
		else if(Token == "-port" && TokensRemaining > 0) {
			NetworkPort = (uint16_t)atoi(Arguments[++i]);
		}
		else if(Token == "-test") {
			PlayState.IsTesting = true;
		}
	}

	// Initialize network subsystem
	_Network::InitializeSystem();

	// Set random seed
	RandomGenerator.seed(SDL_GetPerformanceCounter());

	// Check state
	if(State == &DedicatedState) {
		Assets.Init(true);
		FrameLimit = new _FrameLimit(DEFAULT_MAXFPS, false);

		DedicatedState.SetNetworkPort(NetworkPort);
	}
	else if(State == &BotState) {
		FrameLimit = new _FrameLimit(DEFAULT_MAXFPS, false);
		BotState.HostAddress = HostAddress;
		BotState.Port = NetworkPort;
	}
	else {

		// Open log
		PlayState.Log.Open((Config.ConfigPath + "client.log").c_str());

		// Initialize SDL
		if(SDL_Init(SDL_INIT_VIDEO) < 0)
			throw std::runtime_error("Failed to initialize SDL");

		// Get fullscreen size
		Config.SetDefaultFullscreenSize();

		// Get window settings
		_WindowSettings WindowSettings;
		WindowSettings.WindowTitle = "choria";
		WindowSettings.Fullscreen = Config.Fullscreen;
		WindowSettings.Vsync = Config.Vsync;
		if(Config.Fullscreen)
			WindowSettings.Size = Config.FullscreenSize;
		else
			WindowSettings.Size = Config.WindowSize;
		WindowSettings.Position = glm::ivec2(SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

		// Set up subsystems
		Graphics.Init(WindowSettings);
		Graphics.SetDepthTest(false);
		Graphics.SetDepthMask(false);
		Assets.Init(false);
		Graphics.SetStaticUniforms();

		FrameLimit = new _FrameLimit(Config.MaxFPS, false);
	}

	Timer = SDL_GetPerformanceCounter();
}

// Shuts down the game
void _Framework::Close() {

	// Close the state
	State->Close();

	// Close subsystems
	Assets.Close();
	Graphics.Close();
	Config.Close();
	delete FrameLimit;

	_Network::CloseSystem();

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
		if(State && FrameworkState == UPDATE) {
			switch(Event.type){
				case SDL_KEYDOWN:
				case SDL_KEYUP: {

					// Handle alt-enter
					if(Event.type == SDL_KEYDOWN && (Event.key.keysym.mod & KMOD_ALT) && Event.key.keysym.scancode == SDL_SCANCODE_RETURN) {
						if(!Event.key.repeat)
							Graphics.ToggleFullScreen(Config.WindowSize, Config.FullscreenSize);
					}
					else {
						_KeyEvent KeyEvent("", Event.key.keysym.scancode, Event.type == SDL_KEYDOWN, Event.key.repeat);
						State->KeyEvent(KeyEvent);
						if(!Event.key.repeat) {
							Actions.InputEvent(_Input::KEYBOARD, Event.key.keysym.scancode, Event.type == SDL_KEYDOWN);
						}
					}
				} break;
				case SDL_TEXTINPUT: {
					_KeyEvent KeyEvent(Event.text.text, 0, 1, 1);
					State->KeyEvent(KeyEvent);
				} break;
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP: {
					_MouseEvent MouseEvent(glm::ivec2(Event.motion.x, Event.motion.y), Event.button.button, Event.type == SDL_MOUSEBUTTONDOWN);
					State->MouseEvent(MouseEvent);
					Actions.InputEvent(_Input::MOUSE_BUTTON, Event.button.button, Event.type == SDL_MOUSEBUTTONDOWN);
				} break;
				case SDL_MOUSEWHEEL: {
					State->MouseWheelEvent(Event.wheel.y);
				} break;
				case SDL_WINDOWEVENT:
					if(Event.window.event)
						State->WindowEvent(Event.window.event);
				break;
				case SDL_QUIT:
					Done = true;
				break;
			}
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
				State->Update(TimeStep);
				TimeStepAccumulator -= TimeStep;
			}

			State->Render(TimeStepAccumulator / TimeStep);
		} break;
		case CLOSE: {
			if(State)
				State->Close();

			State = RequestedState;
			FrameworkState = INIT;
		} break;
	}

	Graphics.Flip(FrameTime);

	if(FrameLimit)
		FrameLimit->Update();
}
