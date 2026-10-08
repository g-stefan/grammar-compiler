// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isMultilineCommentZeroOrMore1(Parser &parser, Token *token, int level);
	static bool isMultilineCommentSequence2(Parser &parser, Token *token, int level);
	static bool isMultilineCommentNot3(Parser &parser, Token *token, int level);

	static bool isMultilineCommentNot3(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();

		if(parser.inputIsToken("*/", TokenType::MultilineComment, node)) {
			parser.inputRestore(position);
			TokenList::destructor(node);
			return false;
		};

		TokenList::destructor(node);
		return true;
	};

	static bool isMultilineCommentSequence2(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::MultilineComment;

		if(isMultilineCommentNot3(parser, node, level)&&
			(isNewLine(parser, node, level)||
			parser.inputIsTokenAny(TokenType::MultilineComment, node))) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	static bool isMultilineCommentZeroOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!(isMultilineCommentSequence2(parser, node, level))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// MultilineComment = "/*" & (!"*/" & (NewLine | .))* & "*/";
	bool isMultilineComment(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::MultilineComment, level, position)) {
			return false;
		};

		if(parser.inputIsToken("/*", TokenType::MultilineComment, node)&&
			isMultilineCommentZeroOrMore1(parser, node, level)&&
			parser.inputIsToken("*/", TokenType::MultilineComment, node)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::MultilineComment, position);
		return false;
	};

};

