// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	// WS = WhiteSpaceOrComment?;
	bool isWS(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::WS, level, position)) {
			return false;
		};

		if((isWhiteSpaceOrComment(parser, node, level) || true)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::WS, position);
		return false;
	};

};

