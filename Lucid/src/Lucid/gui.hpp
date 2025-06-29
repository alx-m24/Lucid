#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <string>
#include <iostream>
#include <filesystem>
#include <type_traits>
#include <unordered_map>

#include "Text/Text.hpp"

namespace Lucid {
	extern std::unordered_map<std::string, std::string*> boundTextInputs;

	void Init(const std::string title);
	void Update();
	void Draw();
	void RequestReDraw();
	void Terminate();

	/* ---Getters--- */
	bool WindowShouldClose();

	/* ---Setters--- */
	template<typename T>
	inline void BindInput(const std::string& id, T* valPointer)
	{
		if constexpr (std::is_same<T, std::string>::value) {
			boundTextInputs[id] = valPointer;
		}
		else {
			static_assert(std::is_same<T, std::string>::value, "Unsupported BindInput type");
		}
	}

	/* ---Callbacks--- */
	void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	void char_callback(GLFWwindow* window, unsigned int codepoint);
	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
}