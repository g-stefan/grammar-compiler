// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isNewLine(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::NewLine)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.inputIsToken("\r\n", TokenType::NewLine, token) ||
		    parser.inputIsToken("\n", TokenType::NewLine, token)) {
			return true;
		};

		parser.memoizationSetFalse(TokenType::NewLine);
		return false;
	};

};
