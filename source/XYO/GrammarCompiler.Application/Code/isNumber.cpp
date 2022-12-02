// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isNumber(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::Number)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if ((parser.input >= '0') && (parser.input <= '9')) {
			Token *node = TokenList::newNode();
			node->type = TokenType::Number;
			node->value += parser.input;

			while (parser.input.read()) {
				if (((parser.input >= '0') && (parser.input <= '9'))) {
					token->value += parser.input;
					continue;
				};
				break;
			};

			TokenList::pushToTail(token, node);
			return true;
		};

		parser.memoizationSetFalse(TokenType::Number);
		return false;
	};
};
