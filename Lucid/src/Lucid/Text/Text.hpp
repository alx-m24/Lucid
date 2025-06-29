#pragma once

#include <stb_truetype.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <unordered_map>

namespace Lucid {
	namespace Text {
		struct Font {
			std::vector<unsigned char> data;
			stbtt_fontinfo info;
		};

		struct Glyph {
			GLuint textureID;
			int width, height;
			int bearingX, bearingY;
			int advance;
		};

		void Init();

		void LoadFont(const std::string& fontPath);
		void GenerateGlyphs(const Font& font, float scale = 1.0f, float fontSize = 32.0f);
		void RenderText(const std::string& text, float windowWidth, float windowHeight, float x, float y, float scale, float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);
	}
}