#pragma once

#include <glm/glm.hpp>
#include <string>

struct Div {
	glm::vec2 position;
	glm::vec2 size;
};

struct TextElement {
	std::string text;
	glm::vec2 position;
	float fontSize;
	std::string font;
	glm::vec4 color;
};
