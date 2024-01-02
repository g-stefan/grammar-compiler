// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_DEBUG_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_DEBUG_HPP

#ifndef XYO_GRAMMARCOMPILER_PARSER_HPP
#	include <XYO/GrammarCompiler/Parser.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Debug {
	void inputPrint(Input &input);
	void tokenPrintType(Token *token);
	void tokenPrint(Token *token, Token *parentScan = nullptr);
	void tokenPrintList(Token *tokenList, Token *parentScan = nullptr);
};

#endif
