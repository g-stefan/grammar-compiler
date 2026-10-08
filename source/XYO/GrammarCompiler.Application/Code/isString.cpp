// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	static bool isStringZeroOrMore1(Parser &parser, Token *token, int level);
	static bool isStringSequence2(Parser &parser, Token *token, int level);
	static bool isStringNot3(Parser &parser, Token *token, int level);
	static bool isStringSequence4(Parser &parser, Token *token, int level);

	static bool isStringNot3(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();

		if(parser.inputIsToken("\"", TokenType::String, node)) {
			parser.inputRestore(position);
			TokenList::destructor(node);
			return false;
		};

		TokenList::destructor(node);
		return true;
	};

	static bool isStringSequence4(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::String;

		if(parser.inputIsToken("\\", TokenType::String, node)&&
			parser.inputIsTokenAny(TokenType::String, node)) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	static bool isStringSequence2(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::String;

		if(isStringNot3(parser, node, level)&&
			(isStringSequence4(parser, node, level)||
			parser.inputIsTokenAny(TokenType::String, node))) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	static bool isStringZeroOrMore1(Parser &parser, Token *token, int level) {
		Token *node = token;
		size_t loopPosition;

		while(!parser.input.isEof()) {
			loopPosition = parser.inputPosition();
			if(!(isStringSequence2(parser, node, level))) {
				break;
			};
			if(parser.inputPosition() == loopPosition) {
				break;
			};
		};
		return true;
	};

	// String = "\"" & (!"\"" & ("\\" & . | .))* & "\"";
	bool isString(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::String, level, position)) {
			return false;
		};

		if(parser.inputIsToken("\"", TokenType::String, node)&&
			isStringZeroOrMore1(parser, node, level)&&
			parser.inputIsToken("\"", TokenType::String, node)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::String, position);
		return false;
	};

};

