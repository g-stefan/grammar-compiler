// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_CODE_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_CODE_CODE_HPP

#ifndef XYO_GRAMMARCOMPILER_PARSER_HPP
#	include <XYO/GrammarCompiler/Parser.hpp>
#endif

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP
#	include <XYO/GrammarCompiler.Application/Code/TokenType.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Code {

	bool isAny(Parser &parser, Token *token, int level);
	bool isBOM(Parser &parser, Token *token, int level);
	bool isNewLine(Parser &parser, Token *token, int level);
	bool isWhiteSpace(Parser &parser, Token *token, int level);
	bool isWhiteSpaceContent(Parser &parser, Token *token, int level);
	bool isWhiteSpaceOrComment(Parser &parser, Token *token, int level);
	bool isNumber(Parser &parser, Token *token, int level);
	bool isString(Parser &parser, Token *token, int level);
	bool isLineComment(Parser &parser, Token *token, int level);
	bool isMultilineComment(Parser &parser, Token *token, int level);
	bool isComment(Parser &parser, Token *token, int level);
	bool isGroup(Parser &parser, Token *token, int level);
	bool isInterval(Parser &parser, Token *token, int level);
	bool isCountInterval(Parser &parser, Token *token, int level);
	bool isExpressionUnaryLeft(Parser &parser, Token *token, int level);
	bool isExpressionTerm(Parser &parser, Token *token, int level);
	bool isExpressionOperator(Parser &parser, Token *token, int level);
	bool isExpression(Parser &parser, Token *token, int level);
	bool isSymbol(Parser &parser, Token *token, int level);
	bool isStatement(Parser &parser, Token *token, int level);

};

#endif
