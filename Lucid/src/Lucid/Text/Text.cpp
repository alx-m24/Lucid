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
        
        const char* vertexCode = """#version 330 core layout(location = 0) in vec3 aPos;     // Vertex position (quad) layout(location = 1) in vec2 aTex; out vec2 TexCoord; uniform mat4 projection; void main() { gl_Position = projection * vec4(aPos, 1.0); TexCoord = aTex; }""";
        const char* fragmentCode = """#version 330 core in vec2 TexCoord; out vec4 FragColor;  uniform sampler2D text; uniform vec3 textColor; // RGB void main() { float alpha = texture(text, TexCoord).r; FragColor = vec4(textColor, alpha); FragColor=vec4(1.0); }""";

		std::unordered_map<std::string, Font> fonts;
		std::unordered_map<char, Glyph> glyphs;
        unsigned int VAO, VBO;
        unsigned int shaderProgram;

        void Init() {
            float quad[6][4] = {}; // will be overwritten
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quad), nullptr, GL_DYNAMIC_DRAW);

            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

            // 2. compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vertexCode, NULL);
            glCompileShader(vertex);
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fragmentCode, NULL);
            glCompileShader(fragment);
            // shader Program
            shaderProgram = glCreateProgram();
            glAttachShader(shaderProgram, vertex);
            glAttachShader(shaderProgram, fragment);
            glLinkProgram(shaderProgram);
            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        void LoadFont(const std::string& fontPath) {
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

            GenerateGlyphs(fonts[fontName]);
        }

        void GenerateGlyphs(const Font& font, float scale, float fontSize)
        {
            for (char c = 32; c < 127; ++c) {
                int width, height, xoff, yoff;
                unsigned char* bitmap = stbtt_GetCodepointBitmap(
                    &font.info, 0, stbtt_ScaleForPixelHeight(&font.info, fontSize),
                    c, &width, &height, &xoff, &yoff
                );

                // Generate OpenGL texture
                GLuint texID;
                glGenTextures(1, &texID);
                glBindTexture(GL_TEXTURE_2D, texID);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                int advance, lsb;
                stbtt_GetCodepointHMetrics(&font.info, c, &advance, &lsb);

                Glyph glyph = {
                    texID,
                    width,
                    height,
                    xoff,
                    yoff,
                    static_cast<int>(advance * scale)
                };

                glyphs[c] = glyph;

                stbtt_FreeBitmap(bitmap, nullptr);
            }

        }

        void RenderText(const std::string& text, float windowWidth, float windowHeight, float x, float y, float scale, float r, float g, float b, float a) {
            for (char c : text) {
                Glyph& glyph = glyphs[c];

                float xpos = x + glyph.bearingX * scale;
                float ypos = y - glyph.bearingY * scale;
                float w = glyph.width * scale;
                float h = glyph.height * scale;

                float vertices[6][4] = {
    { xpos,     ypos + h,   0.0f, 0.0f },
    { xpos,     ypos,       0.0f, 1.0f },
    { xpos + w, ypos,       1.0f, 1.0f },

    { xpos,     ypos + h,   0.0f, 0.0f },
    { xpos + w, ypos,       1.0f, 1.0f },
    { xpos + w, ypos + h,   1.0f, 0.0f }
                };

                glBindTexture(GL_TEXTURE_2D, glyph.textureID);

                // Update VBO
                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

                // Draw textured quad at (xpos, ypos) of size (w, h)
                // Using your quad VAO and g.textureID
                glUseProgram(shaderProgram);
                glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), r, g, b);
                glActiveTexture(GL_TEXTURE0);
                glUniform1i(glGetUniformLocation(shaderProgram, "text"), 0);

                glm::mat4 projection = glm::ortho(0.0f, windowWidth * 1.0f, 0.0f, windowHeight * 1.0f);
                glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

                // Draw
                glDrawArrays(GL_TRIANGLES, 0, 6);

                x += glyph.advance * scale;
            }
        }
	}
}