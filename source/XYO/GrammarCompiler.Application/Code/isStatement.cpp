// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	// @Statement = Symbol & WS & Alternation? & Concatenation? & WS & Assign & WS & Expression & WS & StatementEnd;
	bool isStatement(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Statement, level, position)) {
			return false;
		};

		if(isSymbol(parser, node, level)&&
			isWS(parser, node, level)&&
			(isAlternation(parser, node, level) || true)&&
			(isConcatenation(parser, node, level) || true)&&
			isWS(parser, node, level)&&
			isAssign(parser, node, level)&&
			isWS(parser, node, level)&&
			isExpression(parser, node, level)&&
			isWS(parser, node, level)&&
			isStatementEnd(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Statement, position);
		return false;
	};

};

