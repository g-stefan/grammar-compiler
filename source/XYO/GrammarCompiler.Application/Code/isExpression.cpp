// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isExpressionZeroOrMore1(Parser &parser, Token *token, int level);
	static bool isExpressionSequence2(Parser &parser, Token *token, int level);

	static bool isExpressionSequence2(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::Expression;

		if(isWS(parser, node, level)&&
			(isAlternation(parser, node, level)||
			isConcatenation(parser, node, level))&&
			isWS(parser, node, level)&&
			isExpressionTermExtended(parser, node, level)) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	static bool isExpressionZeroOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!(isExpressionSequence2(parser, node, level))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// Expression = ExpressionTermExtended & (WS & (Alternation | Concatenation) & WS & ExpressionTermExtended)*;
	bool isExpression(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Expression, level, position)) {
			return false;
		};

		if(isExpressionTermExtended(parser, node, level)&&
			isExpressionZeroOrMore1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Expression, position);
		return false;
	};

};

