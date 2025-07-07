#include "Parser.hpp"

namespace Lucid {
	namespace Parser {
		std::map<std::string, glm::vec3> HTMLColors{
			{"red", {1.0f, 0.0f, 0.0f}},
			{"white", {1.0f, 1.0f, 1.0f}}
		};

		std::pair<std::string, std::string> OpenFiles(std::string htmlPath, std::string CSSPath)
		{
			std::ifstream htmlDocument(htmlPath);
			std::stringstream htmlStream;
			htmlStream << htmlDocument.rdbuf();
			std::string htmlData = htmlStream.str();
			std::string CSSData = "";

			htmlDocument.close();

			if (!CSSPath.empty()) {
				std::ifstream CSSDocument(CSSPath);
				std::stringstream CSSStream;
				CSSStream << CSSDocument.rdbuf();
				CSSData = CSSStream.str();

				CSSDocument.close();
			}

			return std::pair<std::string, std::string>(htmlData, CSSData);
		}

		std::shared_ptr<HTMLElement> Parse(std::string htmlPath, std::string CSSPath)
		{
			auto files = OpenFiles(htmlPath, CSSPath);
			std::string HTMLData = files.first;
			std::string CSSData = files.second;

			std::stack<std::shared_ptr<HTMLElement>> parsingStack;
			std::shared_ptr<HTMLElement> root = std::make_shared<HTMLElement>();

			root->tagName = "root";
			parsingStack.push(root);

			unsigned int currentIndex = 0;

			while (MoreTokens(HTMLData, currentIndex)) {
				Token currentToken = getToken(HTMLData, currentIndex);

				if (currentToken.type == Token::Opening) {
					std::shared_ptr<HTMLElement> node = std::make_shared<HTMLElement>();
					node->tagName = currentToken.tagName;
					node->attributes = currentToken.attributes;

					parsingStack.top()->children.push_back(node);
					parsingStack.push(node);
				}
				else if (currentToken.type == Token::Closing){
					if (!parsingStack.empty()) 
						parsingStack.pop();
				}
				else if (currentToken.type == Token::SelfClosing) {
					std::shared_ptr<HTMLElement> node = std::make_shared<HTMLElement>();
					node->tagName = currentToken.tagName;
					node->attributes = currentToken.attributes;

					parsingStack.top()->children.push_back(node);
				}
				else {
					parsingStack.top()->content += currentToken.content;
				}
			}

			processCSS(root, parseCSS(CSSData));

			return root;
		}

		bool MoreTokens(const std::string& text, unsigned int index)
		{
			while (index < text.size()) {
				if (!std::isspace(text[index])) return true;
				++index;
			}
			return false;
		}

		std::vector<CSSRule> parseCSS(const std::string& css) {
			std::vector<CSSRule> rules;

			size_t i = 0;
			while (i < css.size()) {
				// Skip whitespace and comments
				while (i < css.size() && std::isspace(css[i])) ++i;

				// Read selector
				size_t selectorStart = i;
				while (i < css.size() && css[i] != '{') ++i;
				if (i >= css.size()) break;

				std::string selector = css.substr(selectorStart, i - selectorStart);
				selector.erase(std::remove_if(selector.begin(), selector.end(), ::isspace), selector.end());

				++i; // skip '{'
				size_t blockStart = i;

				// Find closing '}'
				while (i < css.size() && css[i] != '}') ++i;
				if (i >= css.size()) break;

				std::string block = css.substr(blockStart, i - blockStart);
				++i; // skip '}'

				// Parse block into key-value pairs
				CSSRule rule;
				rule.selector = selector;

				size_t j = 0;
				while (j < block.size()) {
					// Skip whitespace
					while (j < block.size() && std::isspace(block[j])) ++j;
					size_t keyStart = j;

					while (j < block.size() && block[j] != ':') ++j;
					if (j >= block.size()) break;

					std::string key = block.substr(keyStart, j - keyStart);
					key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());

					++j; // skip ':'
					size_t valStart = j;

					while (j < block.size() && block[j] != ';') ++j;
					std::string value = block.substr(valStart, j - valStart);
					value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());

					rule.properties[key] = value;

					if (j < block.size()) ++j; // skip ';'
				}

				rules.push_back(rule);
			}

			return rules;
		}

		void applyCSSProperties(const std::map<std::string, std::string>& ruleProps, CSSProperties& outProps) {
			for (const auto& [key, value] : ruleProps) {
				if (key == "color") outProps.color = glm::vec4(HTMLColors[value], 1.0f);
				else if (key == "font-size") outProps.fontSize = std::atoi(value.c_str());
				else if (key == "background") outProps.background = glm::vec4(HTMLColors[value], 1.0f);
				else if (key == "padding") outProps.padding = value;
				else if (key == "margin") outProps.margin = value;
				else if (key == "font-weight") outProps.fontWeight = value;
				else if (key == "font-family") outProps.fontfamily = (value.empty()) ? "Arial" : value;
				else if (key == "top") outProps.position.y = std::atof(value.c_str());
				else if (key == "left") outProps.position.x = std::atof(value.c_str());
				// extend for more props
			}
		}

		void applyCSSRulesToNode(std::shared_ptr<HTMLElement>& node, const std::vector<CSSRule>& cssRules) {
			for (const auto& rule : cssRules) {
				const std::string& sel = rule.selector;
				bool matches = false;

				// Match by tag name
				if (sel == node->tagName) matches = true;

				// Match by id
				else if (!node->attributes.Id.empty() && sel == "#" + node->attributes.Id) matches = true;

				// Match by class (supports multiple classes split by space)
				else if (!node->attributes.Class.empty()) {
					std::istringstream ss(node->attributes.Class);
					std::string cls;
					while (ss >> cls) {
						if (sel == "." + cls) {
							matches = true;
							break;
						}
					}
				}

				if (matches) {
					applyCSSProperties(rule.properties, node->properties);
				}
			}
		}

		void processCSS(std::shared_ptr<HTMLElement>& root, const std::vector<CSSRule>& cssRules) {
			if (!root) return;

			applyCSSRulesToNode(root, cssRules);

			for (auto& child : root->children) {
				processCSS(child, cssRules);
			}
		}

		void debug(const std::shared_ptr<HTMLElement>& node, int depth)
		{
			if (!node) return;

			// Indent based on depth
			std::cout << std::string(depth * 2, ' ') << "<" << node->tagName << ", Id: '" << node->attributes.Id << "', Class: '" << node->attributes.Class << "'>";
			if (!node->content.empty())
				std::cout << " " << node->content;
			std::cout << std::endl;

			// Recursively print children
			for (const auto& child : node->children)
				debug(child, depth + 1);
		}

		Token getToken(const std::string& html, unsigned int& index) {
			Token token;

			// Skip whitespace
			while (index < html.size() && std::isspace(html[index])) {
				++index;
			}

			if (html[index] == '<') {
				// It's a tag
				++index;
				if (html[index] == '/') {
					// Closing tag
					++index;
					size_t end = html.find('>', index);
					token.type = Token::Type::Closing;
					token.tagName = html.substr(index, end - index);
					index = end + 1;
				}
				else {
					// Opening or self-closing tag
					size_t end = html.find('>', index);
					bool selfClosing = html[end - 1] == '/';

					std::string inside = html.substr(index, end - index);
					index = end + 1;

					// Remove trailing slash if self-closing
					if (selfClosing && inside.back() == '/') {
						inside.pop_back();
					}

					// Split into tag name and attributes
					std::istringstream ss(inside);
					ss >> token.tagName;
					std::string attr;
					while (ss >> attr) {
						size_t eq = attr.find('=');
						if (eq != std::string::npos) {
							std::string key = attr.substr(0, eq);
							std::string val = attr.substr(eq + 1);
							val.erase(std::remove(val.begin(), val.end(), '"'), val.end());
							if (key == "class") token.attributes.Class = val;
							else if (key == "id") token.attributes.Id = val;
							else std::cout << key << ": " << val << std::endl;
						}
					}

					token.type = selfClosing ? Token::Type::SelfClosing : Token::Type::Opening;
				}
			}
			else {
				// It's text content
				size_t end = html.find('<', index);
				if (end == std::string::npos) end = html.size();
				token.type = Token::Type::Text;
				token.content = html.substr(index, end - index);
				index = end;
			}

			return token;
		}
	}
}