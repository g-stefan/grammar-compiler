// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isString(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::String)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.input == '\"') {
			Token *node = TokenList::newNode();
			node->type = TokenType::String;
			node->value = "";

			node->value += '\"';
			while (parser.input.read()) {
				if (parser.input != '\"') {
					if (parser.input == '\\') {
						node->value += '\\';
						if (parser.input.read()) {
							node->value += parser.input;
							continue;
						} else {
							parser.codeEndFalse(node, TokenType::String);
							return false;
						};
					};

					node->value += parser.input;
				} else {
					node->value += parser.input;
					parser.input.read();

					TokenList::pushToTail(token, node);
					return true;
				};
			};

			parser.inputRestore(node);
			TokenList::destructor(node);
		};

		parser.memoizationSetFalse(TokenType::String);
		return false;
	};

};
