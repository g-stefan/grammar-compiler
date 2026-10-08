// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	// Group = GroupBegin & WS & Expression & WS & GroupEnd;
	bool isGroup(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Group, level, position)) {
			return false;
		};

		if(isGroupBegin(parser, node, level)&&
			isWS(parser, node, level)&&
			isExpression(parser, node, level)&&
			isWS(parser, node, level)&&
			isGroupEnd(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Group, position);
		return false;
	};

};

