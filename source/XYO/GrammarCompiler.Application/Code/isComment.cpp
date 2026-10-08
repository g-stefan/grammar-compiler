// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isCommentOneOrMore1(Parser &parser, Token *token, int level);

	static bool isCommentOneOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		if(!((isLineComment(parser, node, level)||
			isMultilineComment(parser, node, level)))) {
			return false;
		};
		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!((isLineComment(parser, node, level)||
			isMultilineComment(parser, node, level)))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// Comment = (LineComment | MultilineComment)+;
	bool isComment(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Comment, level, position)) {
			return false;
		};

		if(isCommentOneOrMore1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Comment, position);
		return false;
	};

};

