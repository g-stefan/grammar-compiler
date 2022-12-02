// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isWhiteSpaceContent(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::WhiteSpaceContent)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.inputIsOneOrMoreToken(" ", TokenType::WhiteSpaceContent, token) ||
		    parser.inputIsOneOrMoreToken("\t", TokenType::WhiteSpaceContent, token)) {
			return true;
		};

		if (parser.inputIsTokenThenSet(isNewLine, TokenType::WhiteSpaceContent, token, level)) {
			return true;
		};

		if (parser.inputIsTokenThenSet(isBOM, TokenType::WhiteSpaceContent, token, level)) {
			return true;
		};

		parser.memoizationSetFalse(TokenType::WhiteSpaceContent);
		return false;
	};

};
