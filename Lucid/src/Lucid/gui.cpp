#include "gui.hpp"

namespace fs = std::filesystem;

namespace Lucid {
	GLFWwindow* window;
	std::string currentPath;

	bool ShouldRedraw;
	unsigned int SCR_WIDTH = 1000;
	unsigned int SCR_HEIGHT = 600;

	std::string currentFocusInputID;
	std::unordered_map<std::string, std::string*> boundTextInputs;
	std::stack<Div> displayStack;
	std::vector<TextElement> textElements;

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
			Terminate();
			return;
		}

		currentPath = fs::current_path().string() + "\\src\\Lucid\\";

		try {
			Text::Init(currentPath);
			Text::LoadFont("Arial", currentPath + "Fonts\\arial.ttf");
			Text::LoadFont("BRADHITC", currentPath + "Fonts\\BRADHITC.ttf", false);
		}
		catch (const std::exception& ex) {
			std::cout << "\nFailed to init text: " << ex.what() << std::endl;
			Terminate();
			return;
		}

		RequestReDraw();

		LoadPage(fs::current_path().string() + "\\src\\demo.html", fs::current_path().string() + "\\src\\demo.css");
	}

	void Update()
	{
		glfwPollEvents();

		currentFocusInputID = "test";

		if (ShouldRedraw) {
			Draw();
			ShouldRedraw = false;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	void LoadPage(const std::string& htmlPath, std::string CSSPath)
	{
		LoadPage(Parse(htmlPath, CSSPath));
	}

	void LoadPage(const std::shared_ptr<HTMLElement>& node)
	{
		if (!node) return;

		std::cout << "<" << node->tagName << ", Id: '" << node->attributes.Id << "', Class: '" << node->attributes.Class << "'>";
		if (!node->content.empty())
			std::cout << " " << node->content;
		std::cout << std::endl;

		if (node->tagName == "div") {
			Div div;
			div.position = node->properties.position;
			div.size = {};
			displayStack.push(div);
		}
		else if (node->tagName == "p") {
			TextElement& text = textElements.emplace_back();
			text.position = node->properties.position + glm::vec2(0.0f, node->properties.fontSize) + (displayStack.empty() ? glm::vec2(0.0f) : displayStack.top().position);
			text.text = node->content;
			text.color = node->properties.color;
			text.fontSize = node->properties.fontSize;
			text.font = node->properties.fontfamily;
		}

		// Recursively print children
		for (const auto& child : node->children) {
			LoadPage(child);
		}

		if (node->tagName == "div") {
			if (!displayStack.empty()) displayStack.pop();
		}

		RequestReDraw();
	}

	void Draw()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		for (const TextElement text : textElements) {
			Text::setActiveFont(text.font);
			Text::RenderText(text.text, { SCR_WIDTH, SCR_HEIGHT }, text.position, text.fontSize, text.color);
		}

		Text::setActiveFont("BRADHITC");
		Text::RenderText(*boundTextInputs["test"], { SCR_WIDTH, SCR_HEIGHT }, { 50.0f, 500.0f }, boundTextInputs["test"]->length(), {1.0f, 1.0f, 1.0f, 1.0f});

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
		if (key == GLFW_KEY_TAB && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			*boundTextInputs[currentFocusInputID] += "    ";
		}
		if (key == GLFW_KEY_LEFT_SHIFT && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
			Text::setActiveFont("Arial");
		}
	}
}