// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isCountIntervalSequence1(Parser &parser, Token *token, int level);

	static bool isCountIntervalSequence1(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::CountInterval;

		if(isCountIntervalSeparator(parser, node, level)&&
			isWS(parser, node, level)&&
			isNumber(parser, node, level)) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	// CountInterval = CountIntervalBegin & WS & Number & (CountIntervalSeparator & WS & Number)? & WS & CountIntervalEnd;
	bool isCountInterval(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::CountInterval, level, position)) {
			return false;
		};

		if(isCountIntervalBegin(parser, node, level)&&
			isWS(parser, node, level)&&
			isNumber(parser, node, level)&&
			(isCountIntervalSequence1(parser, node, level) || true)&&
			isWS(parser, node, level)&&
			isCountIntervalEnd(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::CountInterval, position);
		return false;
	};

};

