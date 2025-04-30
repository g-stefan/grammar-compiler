// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isWhiteSpaceOrComment(Parser &parser, Token *token, int level) {
		if (parser.memoizationIsFalse(TokenType::WhiteSpaceOrComment)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (isWhiteSpace(parser, token, level)) {
			return true;
		};

		if (isComment(parser, token, level)) {
			return true;
		};

		if (isBOM(parser, token, level)) {
			return true;
		};

		parser.memoizationSetFalse(TokenType::WhiteSpaceOrComment);
		return false;
	};

};
