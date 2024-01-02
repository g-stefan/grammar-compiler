// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isStatement(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Statement, level)) {
			return false;
		};

		if (isSymbol(parser, node, level)) {

			isWhiteSpaceOrComment(parser, node, level);

			if (parser.inputIsToken("|", TokenType::Alternation, node) ||
			    parser.inputIsToken("&", TokenType::Concatenation, node)) {
			};

			isWhiteSpaceOrComment(parser, node, level);

			if (parser.inputIsToken("=", TokenType::Assign, node)) {

				isWhiteSpaceOrComment(parser, node, level);

				if (isExpression(parser, node, level)) {

					isWhiteSpaceOrComment(parser, node, level);

					if (parser.inputIsToken(";", TokenType::StatementEnd, node)) {
						TokenList::pushToTail(token, node);
						return true;
					};
				};
			};
		};

		parser.codeEndFalse(node, TokenType::Statement);
		return false;
	};
};
