// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isSymbolZeroOrMore1(Parser &parser, Token *token, int level);

	static bool isSymbolZeroOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!((parser.inputIsToken("_", TokenType::Symbol, node)||
			parser.inputIsBetween("A", "Z", TokenType::Symbol, node)||
			parser.inputIsBetween("a", "z", TokenType::Symbol, node)||
			parser.inputIsBetween("0", "9", TokenType::Symbol, node)))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// Symbol = "@"? & ("_" | ["A","Z"] | ["a","z"]) & ("_" | ["A","Z"] | ["a","z"] | ["0","9"])*;
	bool isSymbol(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Symbol, level, position)) {
			return false;
		};

		if((parser.inputIsToken("@", TokenType::Symbol, node) || true)&&
			(parser.inputIsToken("_", TokenType::Symbol, node)||
			parser.inputIsBetween("A", "Z", TokenType::Symbol, node)||
			parser.inputIsBetween("a", "z", TokenType::Symbol, node))&&
			isSymbolZeroOrMore1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Symbol, position);
		return false;
	};

};

