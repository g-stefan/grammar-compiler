// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isNumberOneOrMore1(Parser &parser, Token *token, int level);

	static bool isNumberOneOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		if(!(parser.inputIsBetween("0", "9", TokenType::Number, node))) {
			return false;
		};
		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!(parser.inputIsBetween("0", "9", TokenType::Number, node))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// Number = ["0","9"]+;
	bool isNumber(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Number, level, position)) {
			return false;
		};

		if(isNumberOneOrMore1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Number, position);
		return false;
	};

};

