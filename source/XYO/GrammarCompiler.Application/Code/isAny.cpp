// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isAny(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Any, level)) {
			return false;
		};

		if (parser.inputIsToken(".", TokenType::Any, node)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Any);
		return false;
	};

};
