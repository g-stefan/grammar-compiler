// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isExpressionTermExtendedSequence1(Parser &parser, Token *token, int level);

	static bool isExpressionTermExtendedSequence1(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::ExpressionTermExtended;

		if(isWS(parser, node, level)&&
			(isZeroOrOne(parser, node, level)||
			isZeroOrMore(parser, node, level)||
			isOneOrMore(parser, node, level)||
			isCountInterval(parser, node, level))) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	// ExpressionTermExtended = ExpressionTerm & (WS & (ZeroOrOne | ZeroOrMore | OneOrMore | CountInterval))?;
	bool isExpressionTermExtended(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::ExpressionTermExtended, level, position)) {
			return false;
		};

		if(isExpressionTerm(parser, node, level)&&
			(isExpressionTermExtendedSequence1(parser, node, level) || true)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::ExpressionTermExtended, position);
		return false;
	};

};

