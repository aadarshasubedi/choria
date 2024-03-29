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
#include <config.h>
#include <files.h>
#include <constants.h>
#include <actions.h>
#include <input.h>
#include <sstream>
#include <fstream>
#include <SDL_filesystem.h>
#include <SDL_video.h>

// Globals
_Config Config;

// Initializes the config system
void _Config::Init(const std::string &ConfigFile) {

	// Create config path
	char *PrefPath = SDL_GetPrefPath("", "choria");
	if(PrefPath) {
		ConfigPath = PrefPath;
		SDL_free(PrefPath);
	}
	else {
		throw std::runtime_error("Cannot create config path!");
	}

	this->ConfigFile = ConfigPath + ConfigFile;

	// Load defaults
	SetDefaults();

	// Load config
	Load();
}

// Closes the config system
void _Config::Close() {
}

// Set defaults
void _Config::SetDefaults() {

	// Gets set after SDL
	FullscreenSize = glm::ivec2(0, 0);

	// Set defaults
	Version = DEFAULT_CONFIG_VERSION;
	WindowSize = DEFAULT_WINDOW_SIZE;
	MSAA = 0;
	Anisotropy = DEFAULT_ANISOTROPY;
	Fullscreen = DEFAULT_FULLSCREEN;
	Vsync = DEFAULT_VSYNC;
	MaxFPS = DEFAULT_MAXFPS;
	AudioEnabled = DEFAULT_AUDIOENABLED;

	SoundVolume = 1.0f;
	MusicVolume = 1.0f;

	MaxClients = DEFAULT_MAXCLIENTS;
	FakeLag = 0.0f;
	NetworkRate = DEFAULT_NETWORKRATE;
	NetworkPort = DEFAULT_NETWORKPORT;

#ifdef _WIN32
	BrowserCommand = "explorer";
#else
	BrowserCommand = "xdg-open";
#endif

	ShowTutorial = 1;

	DesignToolURL = "http://localhost:8000";
	LastHost = "127.0.0.1";
	LastPort = std::to_string(DEFAULT_NETWORKPORT);

	LoadDefaultInputBindings();
}

// Load default key bindings
void _Config::LoadDefaultInputBindings() {

	// Clear mappings
	for(int i = 0; i < _Input::INPUT_COUNT; i++)
		Actions.ClearMappings(i);

	// Movement
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_UP, _Actions::UP, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_E, _Actions::UP, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_DOWN, _Actions::DOWN, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_D, _Actions::DOWN, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_LEFT, _Actions::LEFT, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_S, _Actions::LEFT, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_RIGHT, _Actions::RIGHT, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_F, _Actions::RIGHT, 1.0f, -1.0f, false);

	// Misc
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_ESCAPE, _Actions::MENU, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_C, _Actions::INVENTORY, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_R, _Actions::SKILLS, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_Q, _Actions::TELEPORT, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_T, _Actions::TRADE, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_RETURN, _Actions::CHAT, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_ENTER, _Actions::CHAT, 1.0f, -1.0f, false);

	// Skills
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_1, _Actions::SKILL1, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_2, _Actions::SKILL2, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_3, _Actions::SKILL3, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_4, _Actions::SKILL4, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_5, _Actions::SKILL5, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_6, _Actions::SKILL6, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_7, _Actions::SKILL7, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_8, _Actions::SKILL8, 1.0f, -1.0f, false);

	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_1, _Actions::SKILL1, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_2, _Actions::SKILL2, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_3, _Actions::SKILL3, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_4, _Actions::SKILL4, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_5, _Actions::SKILL5, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_6, _Actions::SKILL6, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_7, _Actions::SKILL7, 1.0f, -1.0f, false);
	Actions.AddInputMap(_Input::KEYBOARD, SDL_SCANCODE_KP_8, _Actions::SKILL8, 1.0f, -1.0f, false);

}

// Use SDL to determine desktop size
void _Config::SetDefaultFullscreenSize() {
	if(Config.FullscreenSize != glm::ivec2(0))
		return;

	SDL_DisplayMode DisplayMode;
	if(SDL_GetDesktopDisplayMode(0, &DisplayMode) != 0)
		Config.FullscreenSize = DEFAULT_WINDOW_SIZE;
	else
		Config.FullscreenSize = glm::ivec2(DisplayMode.w, DisplayMode.h);

	Config.Save();
}

// Load the config file
void _Config::Load() {

	// Open file
	std::ifstream File(ConfigFile.c_str());
	if(!File) {
		Save();
		return;
	}

	// Read data into map
	Map.clear();
	char Buffer[256];
	while(File) {
		File.getline(Buffer, 256);
		if(File.good()) {
			std::string Line(Buffer);
			std::size_t Pos = Line.find_first_of('=');
			if(Pos != std::string::npos) {
				std::string Field = Line.substr(0, Pos);
				std::string Value = Line.substr(Pos+1, Line.size());

				Map[Field].push_back(Value);
			}
		}
	}

	// Close
	File.close();

	// Read version
	int ReadVersion = 0;
	GetValue("version", ReadVersion);
	if(ReadVersion != Version) {
		std::rename(ConfigFile.c_str(), (ConfigFile + "." + std::to_string(ReadVersion)).c_str());
		Save();
		return;
	}

	// Read config
	GetValue("window_width", WindowSize.x);
	GetValue("window_height", WindowSize.y);
	GetValue("fullscreen_width", FullscreenSize.x);
	GetValue("fullscreen_height", FullscreenSize.y);
	GetValue("fullscreen", Fullscreen);
	GetValue("vsync", Vsync);
	GetValue("max_fps", MaxFPS);
	GetValue("anisotropy", Anisotropy);
	GetValue("msaa", MSAA);
	GetValue("audio_enabled", AudioEnabled);
	GetValue("sound_volume", SoundVolume);
	GetValue("music_volume", MusicVolume);
	GetValue("fake_lag", FakeLag);
	GetValue("max_clients", MaxClients);
	GetValue("network_rate", NetworkRate);
	GetValue("network_port", NetworkPort);
	GetValue("browser_command", BrowserCommand);
	GetValue("designtool_url", DesignToolURL);
	GetValue("showtutorial", ShowTutorial);
	GetValue("last_host", LastHost);
	GetValue("last_port", LastPort);

	if(NetworkRate < 0.01)
		NetworkRate = 0.01;

	// Clear bindings
	for(int i = 0; i < _Input::INPUT_COUNT; i++)
		Actions.ClearMappings(i);

	// Load bindings
	for(int i = 0; i < _Actions::COUNT; i++) {
		std::ostringstream Buffer;
		Buffer << "action_" << i;

		// Get list of inputs for each action
		const auto &Values = Map[Buffer.str()];
		for(auto &Iterator : Values) {

			// Parse input bind
			int InputType, Input;
			char Dummy;
			std::stringstream Stream(Iterator);
			Stream >> InputType >> Dummy >> Input;
			Actions.AddInputMap(InputType, Input, i, 1.0f, -1.0f, false);
		}
	}
}

// Save variables to the config file
void _Config::Save() {

	// Open file
	std::ofstream File(ConfigFile.c_str());
	if(!File.is_open())
		return;

	// Write variables
	File << "version=" << Version << std::endl;
	File << "window_width=" << WindowSize.x << std::endl;
	File << "window_height=" << WindowSize.y << std::endl;
	File << "fullscreen_width=" << FullscreenSize.x << std::endl;
	File << "fullscreen_height=" << FullscreenSize.y << std::endl;
	File << "fullscreen=" << Fullscreen << std::endl;
	File << "vsync=" << Vsync << std::endl;
	File << "max_fps=" << MaxFPS << std::endl;
	File << "msaa=" << MSAA << std::endl;
	File << "anisotropy=" << Anisotropy << std::endl;
	File << "audio_enabled=" << AudioEnabled << std::endl;
	File << "sound_volume=" << SoundVolume << std::endl;
	File << "music_volume=" << MusicVolume << std::endl;
	File << "fake_lag=" << FakeLag << std::endl;
	File << "max_clients=" << MaxClients << std::endl;
	File << "network_rate=" << NetworkRate << std::endl;
	File << "network_port=" << NetworkPort << std::endl;
	File << "browser_command=" << BrowserCommand << std::endl;
	File << "designtool_url=" << DesignToolURL << std::endl;
	File << "showtutorial=" << ShowTutorial << std::endl;
	File << "last_host=" << LastHost << std::endl;
	File << "last_port=" << LastPort << std::endl;

	// Write out input map
	Actions.Serialize(File, _Input::KEYBOARD);
	Actions.Serialize(File, _Input::MOUSE_AXIS);
	Actions.Serialize(File, _Input::MOUSE_BUTTON);

	File.close();
}
