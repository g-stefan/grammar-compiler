// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isComment(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Comment, level)) {
			return false;
		};

		if (isLineComment(parser, node, level) || isMultilineComment(parser, node, level)) {
			while (isLineComment(parser, node, level) || isMultilineComment(parser, node, level)) {
				//
			};
			TokenList::pushToTail(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Comment);
		return false;
	};

};
