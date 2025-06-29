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

#include "Shader/Shader.hpp"

namespace Lucid {
	namespace Text {
		struct Font {
			std::string name;
			std::vector<unsigned char> data;
			stbtt_fontinfo info;
		};

		struct Glyph {
			GLuint textureID;
			int width, height;
			int bearingX, bearingY;
			int advance;
		};

		void Init(const std::string& currentPath);

		void LoadFont(const std::string& fontPath, bool setActive = true);
		void setActiveFont(const std::string& fontName);
		void GenerateGlyphs(const std::string& fontName, float fontSize = 32.0f);
		void RenderText(const std::string& text, glm::vec2 windowSize, glm::vec2 position, float fontSize, glm::vec4 color);
		float MeasureTextWidth(const std::string& text, const std::string& fontName, float fontSize);
	}
}