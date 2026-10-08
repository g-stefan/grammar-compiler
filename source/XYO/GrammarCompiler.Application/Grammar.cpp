// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Grammar.hpp>
#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application {

	GrammarNode::GrammarNode() {
		activeConstructor();
	};

	GrammarNode::~GrammarNode(){};

};

namespace XYO::GrammarCompiler::Application::Grammar {

	using namespace XYO::GrammarCompiler::Application::Code;

	typedef TRedBlackTree<String, GrammarNode *> RuleMap;

	GrammarNode *newNode(uint16_t type) {
		GrammarNode *node = GrammarNodeList::newNode();
		node->type = type;
		return node;
	};

	GrammarNode *newNode(uint16_t type, GrammarNode *child) {
		GrammarNode *node = newNode(type);
		GrammarNodeList::pushToTail(node, child);
		return node;
	};

	GrammarNode *newNode(uint16_t type, GrammarNode *child1, GrammarNode *child2) {
		GrammarNode *node = newNode(type);
		GrammarNodeList::pushToTail(node, child1);
		GrammarNodeList::pushToTail(node, child2);
		return node;
	};

	void release(GrammarNode *node) {
		if (node != nullptr) {
			GrammarNodeList::destructor(node);
		};
	};

	void moveChildren(GrammarNode *to, GrammarNode *from) {
		GrammarNode *child;
		while ((child = GrammarNodeList::pop(from)) != nullptr) {
			GrammarNodeList::pushToTail(to, child);
		};
	};

	size_t childCount(GrammarNode *node) {
		size_t count = 0;
		GrammarNode *scan;
		for (scan = node->childHead; scan != nullptr; scan = scan->next) {
			++count;
		};
		return count;
	};

	GrammarNode *copy(GrammarNode *node) {
		GrammarNode *retV = newNode(node->type);
		retV->value = node->value;
		retV->value2 = node->value2;
		retV->min = node->min;
		retV->max = node->max;
		retV->rule = node->rule;
		GrammarNode *scan;
		for (scan = node->childHead; scan != nullptr; scan = scan->next) {
			GrammarNodeList::pushToTail(retV, copy(scan));
		};
		return retV;
	};

	static bool isEqualLiteral(const String &a, const String &b) {
		std::string valueA;
		std::string valueB;
		if (decodeLiteral(a, valueA) && decodeLiteral(b, valueB)) {
			return (valueA == valueB);
		};
		return (a == b);
	};

	bool isEqual(GrammarNode *a, GrammarNode *b) {
		if (a->type != b->type) {
			return false;
		};
		switch (a->type) {
		case GrammarNodeType::Literal:
			return isEqualLiteral(a->value, b->value);
		case GrammarNodeType::Range:
			return isEqualLiteral(a->value, b->value) && isEqualLiteral(a->value2, b->value2);
		case GrammarNodeType::Reference:
			return (a->value == b->value);
		case GrammarNodeType::Repeat:
			if ((a->min != b->min) || (a->max != b->max)) {
				return false;
			};
			break;
		default:
			break;
		};
		GrammarNode *scanA = a->childHead;
		GrammarNode *scanB = b->childHead;
		for (; (scanA != nullptr) && (scanB != nullptr); scanA = scanA->next, scanB = scanB->next) {
			if (!isEqual(scanA, scanB)) {
				return false;
			};
		};
		return (scanA == nullptr) && (scanB == nullptr);
	};

	// C string literal, with quotes, to bytes
	bool decodeLiteral(const String &literal, std::string &output) {
		const char *value = literal.value();
		size_t length = literal.length();
		size_t k;
		output.clear();
		if ((length < 2) || (value[0] != '\"') || (value[length - 1] != '\"')) {
			return false;
		};
		for (k = 1; k < length - 1; ++k) {
			char ch = value[k];
			if (ch != '\\') {
				output += ch;
				continue;
			};
			++k;
			if (k >= length - 1) {
				return false;
			};
			ch = value[k];
			switch (ch) {
			case 'a':
				output += '\a';
				continue;
			case 'b':
				output += '\b';
				continue;
			case 'f':
				output += '\f';
				continue;
			case 'n':
				output += '\n';
				continue;
			case 'r':
				output += '\r';
				continue;
			case 't':
				output += '\t';
				continue;
			case 'v':
				output += '\v';
				continue;
			case '\\':
			case '\'':
			case '\"':
			case '?':
				output += ch;
				continue;
			case 'x': {
				unsigned int code = 0;
				size_t count = 0;
				for (; k + 1 < length - 1; ++k, ++count) {
					char hex = value[k + 1];
					if ((hex >= '0') && (hex <= '9')) {
						code = code * 16 + (hex - '0');
					} else if ((hex >= 'a') && (hex <= 'f')) {
						code = code * 16 + (hex - 'a' + 10);
					} else if ((hex >= 'A') && (hex <= 'F')) {
						code = code * 16 + (hex - 'A' + 10);
					} else {
						break;
					};
					if (code > 255) {
						return false;
					};
				};
				if (count == 0) {
					return false;
				};
				output += static_cast<char>(code);
				continue;
			};
			default:
				break;
			};
			if ((ch >= '0') && (ch <= '7')) {
				unsigned int code = ch - '0';
				size_t count;
				for (count = 1; (count < 3) && (k + 1 < length - 1); ++count) {
					char octal = value[k + 1];
					if ((octal < '0') || (octal > '7')) {
						break;
					};
					code = code * 8 + (octal - '0');
					++k;
				};
				if (code > 255) {
					return false;
				};
				output += static_cast<char>(code);
				continue;
			};
			return false;
		};
		return true;
	};

	String encodeLiteral(const std::string &value) {
		String retV = "\"";
		size_t k;
		for (k = 0; k < value.length(); ++k) {
			uint8_t ch = static_cast<uint8_t>(value[k]);
			switch (ch) {
			case '\"':
				retV += "\\\"";
				continue;
			case '\\':
				retV += "\\\\";
				continue;
			case '\r':
				retV += "\\r";
				continue;
			case '\n':
				retV += "\\n";
				continue;
			case '\t':
				retV += "\\t";
				continue;
			default:
				break;
			};
			if ((ch >= 0x20) && (ch < 0x7F)) {
				retV += static_cast<char>(ch);
				continue;
			};
			char buffer[8];
			snprintf(buffer, sizeof(buffer), "\\%03o", ch);
			retV += buffer;
		};
		retV += "\"";
		return retV;
	};

	static bool isAtom(GrammarNode *node) {
		switch (node->type) {
		case GrammarNodeType::Empty:
		case GrammarNodeType::Literal:
		case GrammarNodeType::Range:
		case GrammarNodeType::Any:
		case GrammarNodeType::Reference:
		case GrammarNodeType::Not:
			return true;
		default:
			break;
		};
		return false;
	};

	static void print(GrammarNode *node, String &output);

	static void printChild(GrammarNode *node, bool parentheses, String &output) {
		if (parentheses) {
			output += "(";
			print(node, output);
			output += ")";
			return;
		};
		print(node, output);
	};

	static void print(GrammarNode *node, String &output) {
		GrammarNode *scan;
		switch (node->type) {
		case GrammarNodeType::Empty:
			output += "\"\"";
			return;
		case GrammarNodeType::Literal:
			output += node->value;
			return;
		case GrammarNodeType::Range:
			output << "[" << node->value << "," << node->value2 << "]";
			return;
		case GrammarNodeType::Any:
			output += ".";
			return;
		case GrammarNodeType::Reference:
			output += node->value;
			return;
		case GrammarNodeType::Sequence:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (scan != node->childHead) {
					output += " & ";
				};
				printChild(scan, (scan->type == GrammarNodeType::Choice) || (scan->type == GrammarNodeType::Sequence), output);
			};
			return;
		case GrammarNodeType::Choice:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (scan != node->childHead) {
					output += " | ";
				};
				printChild(scan, (scan->type == GrammarNodeType::Choice), output);
			};
			return;
		case GrammarNodeType::Optional:
			printChild(node->childHead, !isAtom(node->childHead), output);
			output += "?";
			return;
		case GrammarNodeType::ZeroOrMore:
			printChild(node->childHead, !isAtom(node->childHead), output);
			output += "*";
			return;
		case GrammarNodeType::OneOrMore:
			printChild(node->childHead, !isAtom(node->childHead), output);
			output += "+";
			return;
		case GrammarNodeType::Repeat: {
			char buffer[64];
			printChild(node->childHead, !isAtom(node->childHead), output);
			if (node->max < 0) {
				snprintf(buffer, sizeof(buffer), "{%d}", node->min);
			} else {
				snprintf(buffer, sizeof(buffer), "{%d,%d}", node->min, node->max);
			};
			output += buffer;
			return;
		};
		case GrammarNodeType::Not:
			output += "!";
			printChild(node->childHead, !isAtom(node->childHead), output);
			return;
		default:
			break;
		};
		output += "?unknown?";
	};

	String toString(GrammarNode *node) {
		String retV;
		print(node, retV);
		return retV;
	};

	String toStringRule(GrammarNode *rule, GrammarNode *expression) {
		String retV;
		if (rule->isParserTable) {
			retV += "@";
		};
		retV << rule->value << " = " << toString(expression) << ";";
		return retV;
	};

	String toStringGrammar(GrammarNode *grammar) {
		String retV;
		GrammarNode *rule;
		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			retV << toStringRule(rule, rule->childHead) << "\r\n";
		};
		return retV;
	};

	//
	// Build from the parse tree
	//

	static String symbolName(const String &symbol) {
		if (symbol[0] == '@') {
			return symbol.index(1);
		};
		return symbol;
	};

	// The name is used as TokenType::name in the generated code
	static bool checkName(const String &name, String &error) {
		static const char *keyword[] = {
		    "alignas", "alignof", "and", "and_eq", "asm", "auto", "bitand", "bitor",
		    "bool", "break", "case", "catch", "char", "char8_t", "char16_t", "char32_t",
		    "class", "compl", "concept", "const", "consteval", "constexpr", "constinit", "const_cast",
		    "continue", "co_await", "co_return", "co_yield", "decltype", "default", "delete", "do",
		    "double", "dynamic_cast", "else", "enum", "explicit", "export", "extern", "false",
		    "float", "for", "friend", "goto", "if", "inline", "int", "long",
		    "mutable", "namespace", "new", "noexcept", "not", "not_eq", "nullptr", "operator",
		    "or", "or_eq", "private", "protected", "public", "register", "reinterpret_cast", "requires",
		    "return", "short", "signed", "sizeof", "static", "static_assert", "static_cast", "struct",
		    "switch", "template", "this", "thread_local", "throw", "true", "try", "typedef",
		    "typeid", "typename", "union", "unsigned", "using", "virtual", "void", "volatile",
		    "wchar_t", "while", "xor", "xor_eq",
		    nullptr};
		const char **scan;
		if (name == "Unknown") {
			error = "Symbol Unknown is reserved, TokenType::Unknown is the type of no symbol";
			return false;
		};
		for (scan = keyword; *scan != nullptr; ++scan) {
			if (name == *scan) {
				error = "Symbol ";
				error << name << " is a C++ keyword";
				return false;
			};
		};
		return true;
	};

	// The generated code compares input with strlen, a NUL byte would end the literal
	static bool checkNoNul(const std::string &value, const String &literal, const String &name, String &error) {
		if (value.find('\0') == std::string::npos) {
			return true;
		};
		error = "String ";
		error << literal << " contains a NUL byte, symbol " << name;
		return false;
	};

	static Token *findChild(Token *token, Token::Type type) {
		Token *scan;
		for (scan = token->childHead; scan != nullptr; scan = scan->next) {
			if (scan->type == type) {
				return scan;
			};
		};
		return nullptr;
	};

	static bool getCount(Token *number, int &count, String &error) {
		const char *scan = number->value.value();
		count = 0;
		if (*scan == 0) {
			error = "Invalid number";
			return false;
		};
		for (; *scan != 0; ++scan) {
			if ((*scan < '0') || (*scan > '9')) {
				error = "Invalid number ";
				error += number->value;
				return false;
			};
			count = count * 10 + (*scan - '0');
			if (count > 1000000) {
				error = "Number too big ";
				error += number->value;
				return false;
			};
		};
		return true;
	};

	// Single element sequence or choice is the element
	static GrammarNode *unwrap(GrammarNode *node) {
		if ((node->type == GrammarNodeType::Sequence) || (node->type == GrammarNodeType::Choice)) {
			if ((node->childHead != nullptr) && (node->childHead == node->childTail)) {
				GrammarNode *child = GrammarNodeList::pop(node);
				release(node);
				return child;
			};
		};
		return node;
	};

	static GrammarNode *buildExpression(Token *expression, const String &name, String &error);

	static GrammarNode *buildTerm(Token *term, const String &name, String &error) {
		Token *scan;
		GrammarNode *node;
		if (term == nullptr) {
			error = "Invalid expression for symbol ";
			error += name;
			return nullptr;
		};
		for (scan = term->childHead; scan != nullptr; scan = scan->next) {
			switch (scan->type) {
			case TokenType::Any:
				return newNode(GrammarNodeType::Any);
			case TokenType::Symbol:
				node = newNode(GrammarNodeType::Reference);
				node->value = symbolName(scan->value);
				return node;
			case TokenType::String: {
				std::string value;
				if (decodeLiteral(scan->value, value) && !checkNoNul(value, scan->value, name, error)) {
					return nullptr;
				};
				node = newNode(GrammarNodeType::Literal);
				node->value = scan->value;
				return node;
			};
			case TokenType::Interval: {
				Token *string1 = findChild(scan, TokenType::String);
				Token *string2 = nullptr;
				if (string1 != nullptr) {
					for (string2 = string1->next; string2 != nullptr; string2 = string2->next) {
						if (string2->type == TokenType::String) {
							break;
						};
					};
				};
				if (string2 == nullptr) {
					error = "Invalid interval for symbol ";
					error += name;
					return nullptr;
				};
				std::string value1;
				std::string value2;
				if (!decodeLiteral(string1->value, value1) || !decodeLiteral(string2->value, value2) || (value1.length() == 0) || (value1.length() != value2.length())) {
					error = "Interval [";
					error << string1->value << "," << string2->value << "] strings must be not empty and of the same length, symbol " << name;
					return nullptr;
				};
				if (!checkNoNul(value1, string1->value, name, error) || !checkNoNul(value2, string2->value, name, error)) {
					return nullptr;
				};
				node = newNode(GrammarNodeType::Range);
				node->value = string1->value;
				node->value2 = string2->value;
				return node;
			};
			case TokenType::Group:
				return buildExpression(findChild(scan, TokenType::Expression), name, error);
			case TokenType::Negation: {
				Token *next;
				for (next = scan->next; next != nullptr; next = next->next) {
					if (next->type == TokenType::ExpressionTerm) {
						break;
					};
				};
				node = buildTerm(next, name, error);
				if (node == nullptr) {
					return nullptr;
				};
				return newNode(GrammarNodeType::Not, node);
			};
			default:
				break;
			};
		};
		error = "Invalid expression for symbol ";
		error += name;
		return nullptr;
	};

	static GrammarNode *buildTermExtended(Token *termExtended, const String &name, String &error) {
		GrammarNode *node = buildTerm(findChild(termExtended, TokenType::ExpressionTerm), name, error);
		if (node == nullptr) {
			return nullptr;
		};
		Token *scan;
		for (scan = termExtended->childHead; scan != nullptr; scan = scan->next) {
			switch (scan->type) {
			case TokenType::ZeroOrOne:
				return newNode(GrammarNodeType::Optional, node);
			case TokenType::ZeroOrMore:
				return newNode(GrammarNodeType::ZeroOrMore, node);
			case TokenType::OneOrMore:
				return newNode(GrammarNodeType::OneOrMore, node);
			case TokenType::CountInterval: {
				Token *number1 = findChild(scan, TokenType::Number);
				Token *number2 = nullptr;
				int count1 = 0;
				int count2 = -1;
				if (number1 != nullptr) {
					for (number2 = number1->next; number2 != nullptr; number2 = number2->next) {
						if (number2->type == TokenType::Number) {
							break;
						};
					};
				};
				if ((number1 == nullptr) || !getCount(number1, count1, error)) {
					release(node);
					if (error.length() == 0) {
						error = "Invalid count interval";
					};
					error << " for symbol " << name;
					return nullptr;
				};
				if (number2 != nullptr) {
					if (!getCount(number2, count2, error)) {
						release(node);
						error << " for symbol " << name;
						return nullptr;
					};
					if (count2 < count1) {
						release(node);
						error = "Invalid count interval {";
						error << number1->value << "," << number2->value << "} for symbol " << name;
						return nullptr;
					};
				};
				node = newNode(GrammarNodeType::Repeat, node);
				node->min = count1;
				node->max = count2;
				return node;
			};
			default:
				break;
			};
		};
		return node;
	};

	// Alternation of concatenations, & binds tighter than |
	static GrammarNode *buildExpression(Token *expression, const String &name, String &error) {
		if (expression == nullptr) {
			error = "Invalid expression for symbol ";
			error += name;
			return nullptr;
		};

		GrammarNode *choice = newNode(GrammarNodeType::Choice);
		GrammarNode *sequence = newNode(GrammarNodeType::Sequence);
		Token *scan;

		for (scan = expression->childHead; scan != nullptr; scan = scan->next) {
			if (scan->type == TokenType::ExpressionTermExtended) {
				GrammarNode *term = buildTermExtended(scan, name, error);
				if (term == nullptr) {
					release(choice);
					release(sequence);
					return nullptr;
				};
				GrammarNodeList::pushToTail(sequence, term);
				continue;
			};
			if (scan->type == TokenType::Alternation) {
				GrammarNodeList::pushToTail(choice, unwrap(sequence));
				sequence = newNode(GrammarNodeType::Sequence);
				continue;
			};
		};

		if (sequence->childHead == nullptr) {
			release(choice);
			release(sequence);
			error = "Invalid expression for symbol ";
			error += name;
			return nullptr;
		};

		GrammarNodeList::pushToTail(choice, unwrap(sequence));
		return unwrap(choice);
	};

	bool build(Token *tokenList, GrammarNode *&grammar, String &error) {
		Token *root = TokenList::begin(tokenList);
		Token *statement;
		RuleMap ruleMap;

		grammar = newNode(GrammarNodeType::Grammar);

		for (statement = root->childHead; statement != nullptr; statement = statement->next) {
			if (statement->type != TokenType::Statement) {
				continue;
			};

			Token *symbol = findChild(statement, TokenType::Symbol);
			Token *alternation = findChild(statement, TokenType::Alternation);
			Token *concatenation = findChild(statement, TokenType::Concatenation);
			Token *expression = findChild(statement, TokenType::Expression);
			if ((symbol == nullptr) || (expression == nullptr) || ((alternation != nullptr) && (concatenation != nullptr))) {
				error = "Invalid statement";
				if (symbol != nullptr) {
					error << " for symbol " << symbol->value;
				};
				return false;
			};

			String name = symbolName(symbol->value);
			if (!checkName(name, error)) {
				return false;
			};
			GrammarNode *node = buildExpression(expression, name, error);
			if (node == nullptr) {
				return false;
			};

			RuleMap::Node *found = ruleMap.find(name);

			if ((alternation == nullptr) && (concatenation == nullptr)) {
				if (found != nullptr) {
					release(node);
					error = "Symbol ";
					error << name << " is defined more than once";
					return false;
				};
				GrammarNode *rule = newNode(GrammarNodeType::Rule, node);
				rule->value = name;
				rule->isParserTable = (symbol->value[0] == '@');
				GrammarNodeList::pushToTail(grammar, rule);
				ruleMap.insert(name, rule);
				continue;
			};

			// |= and &=, extend the definition
			if (found == nullptr) {
				release(node);
				error = "Symbol ";
				error << name << " must be defined before using " << (alternation ? "|=" : "&=");
				return false;
			};

			GrammarNode *rule = found->value;
			GrammarNode *previous = GrammarNodeList::pop(rule);
			GrammarNodeList::pushToTail(rule, newNode(alternation ? GrammarNodeType::Choice : GrammarNodeType::Sequence, previous, node));
			if (symbol->value[0] == '@') {
				rule->isParserTable = true;
			};
		};

		GrammarNode *rule;
		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			rule->source = toString(rule->childHead);
		};

		return true;
	};

	static bool resolve(GrammarNode *node, GrammarNode *rule, RuleMap &ruleMap, String &error) {
		if (node->type == GrammarNodeType::Reference) {
			RuleMap::Node *found = ruleMap.find(node->value);
			if (found == nullptr) {
				error = "Symbol ";
				error << node->value << " is not defined, used by " << rule->value;
				return false;
			};
			node->rule = found->value;
			if (node->rule != rule) {
				node->rule->isUsed = true;
			};
		};
		GrammarNode *scan;
		for (scan = node->childHead; scan != nullptr; scan = scan->next) {
			if (!resolve(scan, rule, ruleMap, error)) {
				return false;
			};
		};
		return true;
	};

	bool validate(GrammarNode *grammar, String &error, String &warning) {
		RuleMap ruleMap;
		GrammarNode *rule;
		bool hasParserTable = false;

		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			ruleMap.insert(rule->value, rule);
			if (rule->isParserTable) {
				hasParserTable = true;
			};
		};

		if (!hasParserTable) {
			error = "No symbol is selected for the parser table, use @Symbol";
			return false;
		};

		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			if (!resolve(rule->childHead, rule, ruleMap, error)) {
				return false;
			};
		};

		for (rule = grammar->childHead; rule != nullptr; rule = rule->next) {
			if (!rule->isUsed && !rule->isParserTable) {
				warning << "Warning: symbol " << rule->value << " is not used\r\n";
			};
		};

		return true;
	};

};
