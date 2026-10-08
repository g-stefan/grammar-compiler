// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	// ZeroOrOne = "?";
	bool isZeroOrOne(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::ZeroOrOne, level, position)) {
			return false;
		};

		if(parser.inputIsToken("?", TokenType::ZeroOrOne, node)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::ZeroOrOne, position);
		return false;
	};

};

