// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isExpressionTermSequence1(Parser &parser, Token *token, int level);

	static bool isExpressionTermSequence1(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::ExpressionTerm;

		if(isNegation(parser, node, level)&&
			isWS(parser, node, level)&&
			isExpressionTerm(parser, node, level)) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	// ExpressionTerm = Any | Symbol | String | Interval | Group | Negation & WS & ExpressionTerm;
	bool isExpressionTerm(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::ExpressionTerm, level, position)) {
			return false;
		};

		if(isAny(parser, node, level)||
			isSymbol(parser, node, level)||
			isString(parser, node, level)||
			isInterval(parser, node, level)||
			isGroup(parser, node, level)||
			isExpressionTermSequence1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::ExpressionTerm, position);
		return false;
	};

};

