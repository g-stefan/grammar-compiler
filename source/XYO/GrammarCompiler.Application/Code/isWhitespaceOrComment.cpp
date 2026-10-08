// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isWhiteSpaceOrCommentOneOrMore1(Parser &parser, Token *token, int level);

	static bool isWhiteSpaceOrCommentOneOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		if(!((isWhiteSpace(parser, node, level)||
			isComment(parser, node, level)))) {
			return false;
		};
		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!((isWhiteSpace(parser, node, level)||
			isComment(parser, node, level)))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// @WhiteSpaceOrComment = (WhiteSpace | Comment)+;
	bool isWhiteSpaceOrComment(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::WhiteSpaceOrComment, level, position)) {
			return false;
		};

		if(isWhiteSpaceOrCommentOneOrMore1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::WhiteSpaceOrComment, position);
		return false;
	};

};

