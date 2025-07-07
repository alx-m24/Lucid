#pragma once

#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <vector>
#include <stack>
#include <string>
#include <thread>
#include <map>

#include <glm/glm.hpp>

namespace Lucid {
	namespace Parser {
		struct HTMLAttributes {
			std::string Class;
			std::string Id;
		};
		struct CSSProperties {
			glm::vec4 color;
			unsigned int fontSize;
			glm::vec4 background;
			glm::vec2 position;
			std::string fontfamily;
			std::string padding;
			std::string margin;
			std::string fontWeight;
			// Add more as needed
		};
		struct Token {
			enum Type {
				Opening,
				Closing,
				SelfClosing,
				Text
			} type;
			std::string tagName;
			HTMLAttributes attributes;
			std::string content;
		};

		struct HTMLElement {
			std::string tagName;
			HTMLAttributes attributes;
			CSSProperties properties;
			std::vector<std::shared_ptr<HTMLElement>> children;
			std::string content; // could be text, a link, etc.
		};

		struct CSSRule {
			std::string selector;
			std::map<std::string, std::string> properties;
		};

		std::pair<std::string, std::string> OpenFiles(std::string htmlPath, std::string CSSPath = "");
		// Returns the root of the document
		std::shared_ptr<HTMLElement> Parse(std::string htmlPath, std::string CSSPath = "");
		// Gets the current token and advances the index
		Token getToken(const std::string& text, unsigned int& index);
		// Returns true if there are more tags/tokens after the current index
		bool MoreTokens(const std::string& text, unsigned int index);

		std::vector<CSSRule> parseCSS(const std::string& css);
		void applyCSSProperties(const std::map<std::string, std::string>& ruleProps, CSSProperties& outProps);
		void applyCSSRulesToNode(std::shared_ptr<HTMLElement>& node, const std::vector<CSSRule>& cssRules);
		void processCSS(std::shared_ptr<HTMLElement>& root, const std::vector<CSSRule>& cssRules);


		// Prints all tags from root
		void debug(const std::shared_ptr<HTMLElement>& node, int depth = 0);
	}
}