// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isLineComment(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::LineComment)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.inputIsToken("//", TokenType::LineComment, token) ||
		    parser.inputIsToken("#", TokenType::LineComment, token)) {

			while (!isNewLine(parser, token, level)) {
				if (parser.input.isEof()) {
					return true;
				};
				token->value += parser.input;
				parser.input.read();
			};

			return true;
		};

		parser.memoizationSetFalse(TokenType::LineComment);
		return false;
	};

};
