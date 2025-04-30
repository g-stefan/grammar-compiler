// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isMultilineComment(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::MultilineComment)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.inputIsToken("/*", TokenType::MultilineComment, token)) {
			Token *node = TokenList::newNode();
			node->type = TokenType::MultilineCommentContent;

			while (!parser.inputIs("*/")) {
				if (parser.input.isEof()) {
					TokenList::pushToTail(token, node);
					return true;
				};

				if (isNewLine(parser, node, level)) {
					TokenList::pushToTail(token, node);

					node = TokenList::newNode();
					node->type = TokenType::MultilineCommentContent;
					continue;
				};

				node->value += parser.input;
				parser.input.read();
			};

			TokenList::pushToTail(token, node);

			token->value += "*/";

			return true;
		};

		parser.memoizationSetFalse(TokenType::MultilineComment);
		return false;
	};

};
