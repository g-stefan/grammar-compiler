// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isWhiteSpace(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::WhiteSpace, level)) {
			return false;
		};

		if (isWhiteSpaceContent(parser, node, level)) {

			while (isWhiteSpaceContent(parser, node, level)) {
				//
			};

			TokenList::pushToTail(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::WhiteSpace);
		return false;
	};

};
