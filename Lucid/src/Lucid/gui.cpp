#include "gui.hpp"

namespace fs = std::filesystem;

namespace Lucid {
	GLFWwindow* window;
	std::string currentPath;

	bool ShouldRedraw;
	unsigned int SCR_WIDTH = 1000;
	unsigned int SCR_HEIGHT = 800;

	std::string currentFocusInputID;
	std::unordered_map<std::string, std::string*> boundTextInputs;


	void Init(const std::string title) {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title.c_str(), nullptr, nullptr);
		if (window == nullptr) {
			std::cerr << "Failed to create window" << std::endl;
			return;
		}
		glfwMakeContextCurrent(window);

		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCharCallback(window, char_callback);
		glfwSetKeyCallback(window, key_callback);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cerr << "Failed to initialize GLAD" << std::endl;
			return;
		}

		currentPath = fs::current_path().string() + "\\src\\Lucid\\";

		Text::Init();
		Text::LoadFont(currentPath + "\\Fonts\\arial.ttf");

		RequestReDraw();
	}

	void Update()
	{
		glfwPollEvents();

		currentFocusInputID = "test";

		if (ShouldRedraw) {
			Draw();
			ShouldRedraw = false;
		}
	}

	void Draw()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		Text::RenderText(*boundTextInputs["test"], SCR_WIDTH, SCR_HEIGHT, 0.5f, 0.5f, 1.0f / 17.0f);

		glfwSwapBuffers(window);
	}

	void RequestReDraw()
	{
		ShouldRedraw = true;
	}

	void Terminate()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	bool WindowShouldClose()
	{
		return glfwWindowShouldClose(window);
	}

	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		SCR_WIDTH = width;
		SCR_HEIGHT = height;

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		RequestReDraw();
	}

	void char_callback(GLFWwindow* window, unsigned int codepoint) {
		char enteredChar = static_cast<char>(codepoint);

		if (!currentFocusInputID.empty()) {
			if (enteredChar != 0) {
				*boundTextInputs[currentFocusInputID] += enteredChar;
			}
		}
	}

	void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		RequestReDraw();

		if (currentFocusInputID.empty()) return;

		if (key == GLFW_KEY_BACKSPACE && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			std::string* str = boundTextInputs[currentFocusInputID];
			if (!str->empty()) {
				str->pop_back();
			}
		}
	}
}