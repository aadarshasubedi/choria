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
#include <font.h>
#include <graphics.h>
#include <texture.h>
#include <program.h>
#include <assets.h>
#include <glm/gtc/type_ptr.hpp>
#include <queue>
#include <stdexcept>
#include <cstdint>
#include <functional>
#include <iostream>

// Get next power of two
inline uint32_t GetNextPowerOf2(uint32_t Value) {
	--Value;
	Value |= Value >> 1;
	Value |= Value >> 2;
	Value |= Value >> 4;
	Value |= Value >> 8;
	Value |= Value >> 16;
	return ++Value;
}

// Struct used when sorting glyphs by height
struct CharacterSortStruct {
	FT_UInt Character;
	FT_UInt Height;
};

// Comparison operator for priority queue
bool operator>(const CharacterSortStruct &A, const CharacterSortStruct &B) {
	return A.Height < B.Height;
}

// Constructor
_Font::_Font(const _Program *Program) :
	Program(Program),
	Texture(nullptr) {
}

// Load a font file
_Font::_Font(const std::string &FontFile, const _Program *Program, uint32_t FontSize, uint32_t TextureWidth) :
	_Font(Program) {
	HasKerning = false;
	MaxHeight = 0.0f;
	MaxAbove = 0.0f;
	MaxBelow = 0.0f;

	// Zero out glyphs
	for(int i = 0; i < 256; i++) {
		Glyphs[i].Left = 0.0f;
		Glyphs[i].Top = 0.0f;
		Glyphs[i].Right = 0.0f;
		Glyphs[i].Bottom = 0.0f;
		Glyphs[i].Width = 0.0f;
		Glyphs[i].Height = 0.0f;
		Glyphs[i].Advance = 0.0f;
		Glyphs[i].OffsetX = 0.0f;
		Glyphs[i].OffsetY = 0.0f;
	}

	// Initialize library
	if(FT_Init_FreeType(&Library) != 0) {
		throw std::runtime_error("Error initializing FreeType");
	}

	// Load the font
	if(FT_New_Face(Library, FontFile.c_str(), 0, &Face) != 0) {
		throw std::runtime_error("Error loading font file: " + FontFile);
	}

	// Set font size
	if(FT_Set_Pixel_Sizes(Face, 0, FontSize)) {
		throw std::runtime_error("Error setting pixel size");
	}

	HasKerning = !!FT_HAS_KERNING(Face);
	LoadFlags = FT_LOAD_RENDER;

	// Characters to create
	std::string Characters;
	for(int i = 32; i < 127; i++)
		Characters.push_back(char(i));

	// Sort characters by size
	std::string SortedCharacters;
	SortCharacters(Face, Characters, SortedCharacters);

	// Create the OpenGL texture and populate GlyphUVs
	CreateFontTexture(SortedCharacters, TextureWidth);
}

// Destructor
_Font::~_Font() {

	// Close face
	FT_Done_Face(Face);

	// Close freetype
	FT_Done_FreeType(Library);

	// Free OpenGL texture
	delete Texture;
}

// Sorts characters by vertical size
void _Font::SortCharacters(FT_Face &Face, const std::string &Characters, std::string &SortedCharacters) {

	// Build queue
	std::priority_queue<int, std::vector<CharacterSortStruct>, std::greater<CharacterSortStruct> > CharacterList;
	CharacterSortStruct Character;
	for(size_t i = 0; i < Characters.size(); i++) {

		// Load a character
		FT_Load_Char(Face, (FT_ULong)Characters[i], LoadFlags);
		FT_GlyphSlot &Glyph = Face->glyph;

		// Add character to the list
		Character.Character = (FT_UInt)Characters[i];
		Character.Height = Glyph->bitmap.rows;

		// Save maxes
		if(Character.Height > MaxHeight)
			MaxHeight = Character.Height;
		if(Glyph->bitmap_top > MaxAbove)
			MaxAbove = Glyph->bitmap_top;
		if((float)Glyph->bitmap.rows - Glyph->bitmap_top > MaxBelow)
			MaxBelow = (float)Glyph->bitmap.rows - Glyph->bitmap_top;

		CharacterList.push(Character);
	}

	// Build sorted string
	while(!CharacterList.empty()) {
		const CharacterSortStruct &Character = CharacterList.top();
		SortedCharacters.push_back((char)Character.Character);
		CharacterList.pop();
	}
}

// Renders all the glyphs to a texture
void _Font::CreateFontTexture(std::string SortedCharacters, uint32_t TextureWidth) {
	uint32_t X = 0;
	uint32_t Y = 0;
	uint32_t SpacingX = 1;
	uint32_t SpacingY = 1;
	uint32_t MaxRows = 0;

	// Determine Glyph UVs and texture height given a texture width
	for(size_t i = 0; i < SortedCharacters.size(); i++) {

		// Load a character
		FT_Load_Char(Face, (FT_ULong)SortedCharacters[i], LoadFlags);

		// Get glyph
		FT_GlyphSlot &GlyphSlot = Face->glyph;
		FT_Bitmap *Bitmap = &GlyphSlot->bitmap;

		// Get width and height of glyph
		uint32_t Width = Bitmap->width + SpacingX;
		uint32_t Rows = Bitmap->rows;

		// Start a new line if no room left
		if(X + Width > TextureWidth) {
			X = 0;
			Y += MaxRows + SpacingY;
			MaxRows = 0;
		}

		// Check max values
		if(Rows > MaxRows)
			MaxRows = Rows;

		// Add character to list
		GlyphStruct Glyph;
		Glyph.Left = X;
		Glyph.Top = Y;
		Glyph.Right = X + Bitmap->width;
		Glyph.Bottom = Y + Bitmap->rows;
		Glyph.Width = Bitmap->width;
		Glyph.Height = Bitmap->rows;
		Glyph.Advance = GlyphSlot->advance.x >> 6;
		Glyph.OffsetX = GlyphSlot->bitmap_left;
		Glyph.OffsetY = GlyphSlot->bitmap_top;
		Glyphs[(FT_Byte)SortedCharacters[i]] = Glyph;

		// Update draw position
		X += Width;
	}

	// Add last line
	Y += MaxRows;

	// Round to next power of 2
	uint32_t TextureHeight = GetNextPowerOf2(Y);

	// Create image buffer
	uint32_t TextureSize = TextureWidth * TextureHeight;
	uint8_t *Image = new uint8_t[TextureSize];
	memset(Image, 0, TextureSize);

	// Render each glyph to the texture
	for(size_t i = 0; i < SortedCharacters.size(); i++) {
		GlyphStruct &Glyph = Glyphs[(FT_Byte)SortedCharacters[i]];

		// Load a character
		FT_Load_Char(Face, (FT_ULong)SortedCharacters[i], LoadFlags);

		// Get glyph
		FT_GlyphSlot &GlyphSlot = Face->glyph;
		FT_Bitmap *Bitmap = &GlyphSlot->bitmap;

		// Write character bitmap data
		for(FT_UInt y = 0; y < Bitmap->rows; y++) {

			int DrawY = (int)Glyph.Top + (int)y;
			for(FT_UInt x = 0; x < Bitmap->width; x++) {
				int DrawX = (int)Glyph.Left + (int)x;
				int Destination = DrawX + DrawY * (int)TextureWidth;
				int Source = (int)x + (int)y * Bitmap->pitch;

				// Copy to texture
				Image[Destination] = Bitmap->buffer[Source];
			}
		}

		// Convert Glyph bounding box to UV coords
		Glyph.Left /= (float)TextureWidth;
		Glyph.Top /= (float)TextureHeight;
		Glyph.Right /= (float)TextureWidth;
		Glyph.Bottom /= (float)TextureHeight;
	}

	// Load texture
	Texture = new _Texture(Image, glm::ivec2(TextureWidth, TextureHeight), GL_ALPHA8, GL_ALPHA);

	delete[] Image;
}

// Draw one glyph
void _Font::DrawGlyph(glm::vec2 &Position, char Char, float Scale) const {

	// Get glyph data
	const GlyphStruct &Glyph = Glyphs[(FT_Byte)Char];

	// Get vertices
	glm::vec2 DrawPosition(Position.x + Scale * Glyph.OffsetX, Position.y - Scale * Glyph.OffsetY);
	float Vertices[] = {
		DrawPosition.x,                       DrawPosition.y + Scale * Glyph.Height, Glyph.Left,  Glyph.Bottom,
		DrawPosition.x + Scale * Glyph.Width, DrawPosition.y + Scale * Glyph.Height, Glyph.Right, Glyph.Bottom,
		DrawPosition.x,                       DrawPosition.y,                        Glyph.Left,  Glyph.Top,
		DrawPosition.x + Scale * Glyph.Width, DrawPosition.y,                        Glyph.Right, Glyph.Top,
	};

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, &Vertices[0]);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, &Vertices[2]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	Position.x += Scale * Glyph.Advance;
}

// Adjust position based on alignment
void _Font::AdjustPosition(const std::string &Text, glm::vec2 &Position, bool UseFormatting, const _Alignment &Alignment, float Scale) const {

	// Adjust for alignment
	_TextBounds TextBounds;
	GetStringDimensions(Text, TextBounds, UseFormatting);

	// Handle horizontal alignment
	switch(Alignment.Horizontal) {
		case _Alignment::CENTER:
			Position.x -= Scale * (TextBounds.Width >> 1);
		break;
		case _Alignment::RIGHT:
			Position.x -= Scale * TextBounds.Width;
		break;
	}

	// Handle vertical alignment
	switch(Alignment.Vertical) {
		case _Alignment::TOP:
			Position.y += Scale * TextBounds.AboveBase;
		break;
		case _Alignment::MIDDLE:
			Position.y += Scale * ((TextBounds.AboveBase - TextBounds.BelowBase) >> 1);
		break;
		case _Alignment::BOTTOM:
			Position.y -= Scale * TextBounds.BelowBase;
		break;
	}
}

// Draws a string
float _Font::DrawText(const std::string &Text, glm::vec2 Position, const glm::vec4 &Color, const _Alignment &Alignment, float Scale) const {
	Graphics.SetProgram(Program);
	Graphics.SetVBO(VBO_NONE);
	Graphics.SetColor(Color);
	Graphics.SetTextureID(Texture->ID);

	// Set position
	AdjustPosition(Text, Position, false, Alignment, Scale);

	// Draw string
	glm::vec2 DrawPosition;
	FT_UInt PreviousGlyphIndex = 0;
	for(size_t i = 0; i < Text.size(); i++) {
		FT_UInt GlyphIndex = FT_Get_Char_Index(Face, (FT_ULong)Text[i]);

		// Handle kerning
		if(HasKerning && i) {
			FT_Vector Delta;
			FT_Get_Kerning(Face, PreviousGlyphIndex, GlyphIndex, FT_KERNING_DEFAULT, &Delta);
			Position.x += Scale * (float)(Delta.x >> 6);
		}
		PreviousGlyphIndex = GlyphIndex;

		// Draw glyph
		DrawGlyph(Position, Text[i], Scale);
	}

	return Position.x;
}

// Draw formatted text
void _Font::DrawTextFormatted(const std::string &Text, glm::vec2 Position, const _Alignment &Alignment) const {
	Graphics.SetProgram(Program);
	Graphics.SetVBO(VBO_NONE);
	Graphics.SetColor(glm::vec4(1.0f));
	Graphics.SetTextureID(Texture->ID);
	float Scale = 1.0f;
	bool InTag = false;
	int TagIndex = 0;
	int Mode = 0;
	std::string Attribute = "";

	// Set position
	AdjustPosition(Text, Position, true, Alignment, Scale);

	// Draw string
	FT_UInt PreviousGlyphIndex = 0;
	for(size_t i = 0; i < Text.size(); i++) {
		FT_UInt GlyphIndex = FT_Get_Char_Index(Face, (FT_ULong)Text[i]);

		// Handle kerning
		if(HasKerning && i) {
			FT_Vector Delta;
			FT_Get_Kerning(Face, PreviousGlyphIndex, GlyphIndex, FT_KERNING_DEFAULT, &Delta);
			Position.x += Scale * (float)(Delta.x >> 6);
		}
		PreviousGlyphIndex = GlyphIndex;

		// Get glyph data
		if(Text[i] == '[') {
			InTag = true;
			TagIndex = 0;
		}
		else if(Text[i] == ']') {
			InTag = false;

			if(Mode == 1)
				Graphics.SetColor(Assets.Colors[Attribute]);

			Attribute = "";
			Mode = 0;
		}
		else if(!InTag) {

			// Draw glyph
			DrawGlyph(Position, Text[i], Scale);
		}
		else {

			if(TagIndex == 0) {
				if(Text[i] == 'c')
					Mode = 1;
			}
			else if(TagIndex >= 2 && Mode) {
				Attribute += Text[i];
			}

			TagIndex++;
		}
	}
}

// Get width and height of a string
void _Font::GetStringDimensions(const std::string &Text, _TextBounds &TestBounds, bool UseFormatting) const {
	if(Text.size() == 0) {
		TestBounds.Width = 0;
		TestBounds.AboveBase = 0;
		TestBounds.BelowBase = 0;
		return;
	}

	bool InTag = false;

	TestBounds.Width = TestBounds.AboveBase = TestBounds.BelowBase = 0;
	const GlyphStruct *Glyph = nullptr;
	for(size_t i = 0; i < Text.size(); i++) {

		if(UseFormatting && Text[i] == '[')
			InTag = true;
		else if(UseFormatting && Text[i] == ']')
			InTag = false;
		else if(!InTag) {

			// Get glyph data
			Glyph = &Glyphs[(FT_Byte)Text[i]];

			// Update width by advance
			TestBounds.Width += (int)Glyph->Advance;

			// Get number of pixels below baseline
			int BelowBase = (int)(-Glyph->OffsetY + Glyph->Height);
			if(BelowBase > TestBounds.BelowBase)
				TestBounds.BelowBase = BelowBase;

			// Get number of pixels above baseline
			if(Glyph->OffsetY > (int)TestBounds.AboveBase)
				TestBounds.AboveBase = (int)Glyph->OffsetY;
		}
	}
}

// Break up text into multiple strings based on max width
void _Font::BreakupString(const std::string &Text, float Width, std::list<std::string> &Strings, bool UseFormatting) const {

	bool InTag = false;
	float X = 0;
	FT_UInt PreviousGlyphIndex = 0;
	size_t StartCut = 0;
	size_t LastSpace = std::string::npos;
	for(size_t i = 0; i < Text.size(); i++) {

		// Check for formatting codes
		if(UseFormatting && Text[i] == '[')
			InTag = true;
		else if(UseFormatting && Text[i] == ']')
			InTag = false;
		else if(!InTag) {

			// Remember last space position
			if(Text[i] == ' ')
				LastSpace = i;

			FT_UInt GlyphIndex = FT_Get_Char_Index(Face, (FT_ULong)Text[i]);

			// Handle kerning
			if(HasKerning && i) {
				FT_Vector Delta;
				FT_Get_Kerning(Face, PreviousGlyphIndex, GlyphIndex, FT_KERNING_DEFAULT, &Delta);
				X += (float)(Delta.x >> 6);
			}
			PreviousGlyphIndex = GlyphIndex;

			// Get glyph info
			const GlyphStruct &Glyph = Glyphs[(FT_Byte)Text[i]];
			X += Glyph.Advance;

			// Check for max width
			if(X >= Width) {
				size_t Adjust = 0;
				if(LastSpace == std::string::npos)
					LastSpace = i;
				else
					Adjust = 1;

				// Add to list of strings
				Strings.push_back(Text.substr(StartCut, LastSpace - StartCut));
				StartCut = LastSpace+Adjust;
				LastSpace = std::string::npos;
				i = StartCut;

				// Check for formatting codes
				if(UseFormatting && Text[i] == '[')
					InTag = true;
				else if(UseFormatting && Text[i] == ']')
					InTag = false;

				X = 0;
				PreviousGlyphIndex = 0;
			}
		}
	}

	// Add last cut
	Strings.push_back(Text.substr(StartCut, Text.size()));
}
