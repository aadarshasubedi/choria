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
#include <state.h>
#include <vector>

struct _Tile;
class _Camera;
class _Map;
class _Texture;

enum FilterType {
	FILTER_TEXTURE   = (1 << 1),
	FILTER_WALL      = (1 << 2),
	FILTER_PVP       = (1 << 3),
	FILTER_ZONE      = (1 << 4),
	FILTER_EVENTTYPE = (1 << 5),
	FILTER_EVENTDATA = (1 << 6),
};

// Classes
class _EditorState : public _State {

	public:

		enum ElementType {
			NEWMAP_WINDOW,
			NEWMAP_FILE,
			NEWMAP_WIDTH,
			NEWMAP_HEIGHT,
			NEWMAP_CREATE,
			NEWMAP_CANCEL,
			NEWMAP_ERROR,
			BRUSHOPTIONS_WINDOW,
			BRUSHOPTIONS_WALL,
			BRUSHOPTIONS_PVP,
			BRUSHOPTIONS_ZONE,
			BRUSHOPTIONS_EVENTTYPE,
			BRUSHOPTIONS_EVENTDATA,
			BRUSHOPTIONS_FILTERTEXTURE,
			BRUSHOPTIONS_FILTERWALL,
			BRUSHOPTIONS_FILTERZONE,
			BRUSHOPTIONS_FILTERPVP,
			BRUSHOPTIONS_FILTEREVENTTYPE,
			BRUSHOPTIONS_FILTEREVENTDATA,
			BRUSHOPTIONS_FILTERCLOSE,
			LOADMAP_WINDOW,
			TEXTUREPALETTE_WINDOW,
			TEXTURES_ID=100,
		};

		enum StateType {
			STATE_MAIN,
			STATE_NEWMAP,
			STATE_LOADMAP,
			STATE_TEXTUREPALETTE,
			STATE_BRUSHOPTIONS,
		};

		_EditorState();

		void Init() override;
		void Close() override;

		void Update(double FrameTime) override;
		void Render(double BlendFactor) override;

		void KeyEvent(const _KeyEvent &KeyEvent) override;
		void TextEvent(const char *Text) override;
		void MouseEvent(const _MouseEvent &MouseEvent) override;
		void MouseWheelEvent(int Direction) override;
		void WindowEvent(uint8_t Event) override;

		void SetFilename(const std::string &Filename) { this->Filename = Filename; }

	private:

		void CloseWindow(int TElement);
		void CloseMap();

		// New
		void InitNewMap();
		void CreateMap();

		// Loading
		void InitLoadMap();

		// Texture palette
		void InitTexturePalette();
		void RefreshTexturePalette();

		// Brushes
		void InitBrushOptions();
		void ApplyBrushSize(const glm::vec2 &Position, int BrushSize);
		void ApplyBrush(const glm::vec2 &Position);
		void RenderBrush();

		// General
		int State;

		// Graphics
		_Camera *Camera;
		glm::vec2 WorldCursor;

		// Map
		_Map *Map;
		std::string Filename;
		std::string WorkingDirectory;

		// Textures
		std::vector<_Texture *> TexturePalette;

		// Brush
		int BrushSize;
		_Tile *Brush;

		// Filter
		int Filter;
};

extern _EditorState EditorState;