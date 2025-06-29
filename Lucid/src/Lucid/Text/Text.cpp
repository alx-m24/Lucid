#include "Text.hpp"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

namespace Lucid {
	namespace Text {
#ifdef _WIN32
		int platformID = 3;   // Windows
		int encodingID = 1;   // Unicode BMP (UCS-2)
		int languageID = 0x0409; // English - United States
#elif defined(__APPLE__)
		int platformID = 1;
		int encodingID = 0;
		int languageID = 0;
#elif defined(__linux__)
		int platformID = 1;
		int encodingID = 0;
		int languageID = 0;
#endif

		std::unordered_map<std::string, Font> fonts;
		std::unordered_map<std::string, std::unordered_map<float, std::unordered_map<char, Glyph>>> glyphs;

        std::string activeFont;

        unsigned int VAO, VBO;
        Shader shader;

        void Init(const std::string& currentPath) {
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindVertexArray(0);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            shader = Shader(currentPath + "Text\\Shader\\Text.vert", currentPath + "Text\\Shader\\Text.frag");
        }

        void LoadFont(const std::string& fontPath, bool setActive) {
            // Read font into vector (RAII safe)
            std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
            if (!file) throw std::runtime_error("Failed to open font file");

            std::streamsize size = file.tellg();
            file.seekg(0, std::ios::beg);

            Font font;
            font.data.resize(size);
            if (!file.read(reinterpret_cast<char*>(font.data.data()), size))
                throw std::runtime_error("Failed to read font file");

            if (!stbtt_InitFont(&font.info, font.data.data(), stbtt_GetFontOffsetForIndex(font.data.data(), 0))) {
                throw std::runtime_error("Failed to initialize font");
            }

            int length = 0;
            const char* rawName = stbtt_GetFontNameString(&font.info, &length, platformID, encodingID, languageID, 4);

            std::string fontName = rawName && length > 0 ? std::string(rawName, length) : "UnnamedFont";

            fonts[fontName] = std::move(font);

            if (setActive) setActiveFont(fontName);
        }

        void GenerateGlyphs(const std::string& fontName, float fontSize) {
            if (glyphs.count(fontName)) {
                if (glyphs[fontName].count(fontSize)) return; // Avoid regenerating
            } else throw std::runtime_error("Font not loaded");

            Font& font = fonts[fontName];

            float scale = stbtt_ScaleForPixelHeight(&font.info, fontSize);
            std::unordered_map<char, Glyph> sizeGlyphs;

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            for (char c = 32; c < 127; ++c) {
                int width, height, xoff, yoff;
                unsigned char* bitmap = stbtt_GetCodepointBitmap(&font.info, 0, scale, c, &width, &height, &xoff, &yoff);

                GLuint texID;
                glGenTextures(1, &texID);
                glBindTexture(GL_TEXTURE_2D, texID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                int advance;
                stbtt_GetCodepointHMetrics(&font.info, c, &advance, nullptr);

                int x0, y0, x1, y1;
                stbtt_GetCodepointBitmapBox(&font.info, c, scale, scale, &x0, &y0, &x1, &y1);

                sizeGlyphs[c] = {
                    texID,
                    width,
                    height,
                    x0,
                    -y0,
                    advance
                };

                stbtt_FreeBitmap(bitmap, nullptr);
            }

            glyphs[fontName][fontSize] = std::move(sizeGlyphs);
        }

        void setActiveFont(const std::string& fontName)
        {
            if (fonts.count(fontName) > 0) {
                activeFont = fontName;
            }
            else throw std::runtime_error("Font not loaded");
        }

        void RenderText(const std::string& text, glm::vec2 windowSize, glm::vec2 position, float fontSize, glm::vec4 color) {
            shader.use();
            shader.setMat4("projection", glm::ortho(0.0f, windowSize.x, 0.0f, windowSize.y));
            shader.setVec3("textColor", color);
            glActiveTexture(GL_TEXTURE0);
            glBindVertexArray(VAO);

            if (glyphs[activeFont].count(fontSize) == 0) GenerateGlyphs(activeFont, fontSize);
            auto& glyphMap = glyphs[activeFont][fontSize];

            float scale = stbtt_ScaleForPixelHeight(&fonts[activeFont].info, fontSize);

            // iterate through all characters
            std::string::const_iterator c;
            for (c = text.begin(); c != text.end(); c++)
            {
                Glyph ch = glyphMap[*c];

                float xpos = position.x + ch.bearingX;
                float ypos = position.y - (ch.height - ch.bearingY);

                float w = ch.width;
                float h = ch.height;
                // update VBO for each character
                float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }
                };
                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch.textureID);
                // update content of VBO memory
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // render quad
                glDrawArrays(GL_TRIANGLES, 0, 6);
                // now advance cursors for next glyph
                position.x += ch.advance * scale;
            }
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        float MeasureTextWidth(const std::string& text, const std::string& fontName, float fontSize) {
            float scale = stbtt_ScaleForPixelHeight(&fonts[fontName].info, fontSize);

            if (glyphs[fontName].count(fontSize) == 0)
                GenerateGlyphs(fontName, fontSize);

            float width = 0.0f;
            for (char c : text) {
                width += (glyphs[fontName][fontSize][c].advance >> 6) * scale;
            }
            return width;
        }
	}
}