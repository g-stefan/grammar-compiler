// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isLineCommentZeroOrMore1(Parser &parser, Token *token, int level);
	static bool isLineCommentSequence2(Parser &parser, Token *token, int level);
	static bool isLineCommentNot3(Parser &parser, Token *token, int level);

	static bool isLineCommentNot3(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();

		if(isNewLine(parser, node, level)) {
			parser.inputRestore(position);
			TokenList::destructor(node);
			return false;
		};

		TokenList::destructor(node);
		return true;
	};

	static bool isLineCommentSequence2(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::LineComment;

		if(isLineCommentNot3(parser, node, level)&&
			parser.inputIsTokenAny(TokenType::LineComment, node)) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	static bool isLineCommentZeroOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!(isLineCommentSequence2(parser, node, level))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// LineComment = ("//" | "#") & (!NewLine & .)* & NewLine;
	bool isLineComment(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::LineComment, level, position)) {
			return false;
		};

		if((parser.inputIsToken("//", TokenType::LineComment, node)||
			parser.inputIsToken("#", TokenType::LineComment, node))&&
			isLineCommentZeroOrMore1(parser, node, level)&&
			isNewLine(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::LineComment, position);
		return false;
	};

};

