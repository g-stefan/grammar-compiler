// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	// Interval = IntervalBegin & WS & String & WS & IntervalSeparator & WS & String & WS & IntervalEnd;
	bool isInterval(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Interval, level, position)) {
			return false;
		};

		if(isIntervalBegin(parser, node, level)&&
			isWS(parser, node, level)&&
			isString(parser, node, level)&&
			isWS(parser, node, level)&&
			isIntervalSeparator(parser, node, level)&&
			isWS(parser, node, level)&&
			isString(parser, node, level)&&
			isWS(parser, node, level)&&
			isIntervalEnd(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Interval, position);
		return false;
	};

};

