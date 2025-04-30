// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isSymbol(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::Symbol)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.input == '@' || parser.input == '_' || ((parser.input >= 'A') && (parser.input <= 'Z')) || ((parser.input >= 'a') && (parser.input <= 'z'))) {
			Token *node = TokenList::newNode();
			node->type = TokenType::Symbol;
			node->value += parser.input;
			while (parser.input.read()) {
				if (parser.input == '_' || parser.input == '-' || ((parser.input >= 'A') && (parser.input <= 'Z')) || ((parser.input >= 'a') && (parser.input <= 'z')) || ((parser.input >= '0') && (parser.input <= '9'))) {
					node->value += parser.input;
				} else {
					TokenList::pushToTail(token, node);
					return true;
				};
			};
			TokenList::pushToTail(token, node);
			return true;
		};

		parser.memoizationSetFalse(TokenType::Symbol);
		return false;
	};
};
