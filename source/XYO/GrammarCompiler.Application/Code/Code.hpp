// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_CODE_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_CODE_CODE_HPP

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP
#	include <XYO/GrammarCompiler.Application/Code/TokenType.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	bool isBOM(Parser &parser, Token *token, int level);
	bool isNewLine(Parser &parser, Token *token, int level);
	bool isWhiteSpace(Parser &parser, Token *token, int level);
	bool isLineComment(Parser &parser, Token *token, int level);
	bool isMultilineComment(Parser &parser, Token *token, int level);
	bool isComment(Parser &parser, Token *token, int level);
	bool isWhiteSpaceOrComment(Parser &parser, Token *token, int level);
	bool isString(Parser &parser, Token *token, int level);
	bool isNumber(Parser &parser, Token *token, int level);
	bool isSymbol(Parser &parser, Token *token, int level);
	bool isAlternation(Parser &parser, Token *token, int level);
	bool isConcatenation(Parser &parser, Token *token, int level);
	bool isNegation(Parser &parser, Token *token, int level);
	bool isAny(Parser &parser, Token *token, int level);
	bool isZeroOrOne(Parser &parser, Token *token, int level);
	bool isZeroOrMore(Parser &parser, Token *token, int level);
	bool isOneOrMore(Parser &parser, Token *token, int level);
	bool isIntervalBegin(Parser &parser, Token *token, int level);
	bool isIntervalEnd(Parser &parser, Token *token, int level);
	bool isIntervalSeparator(Parser &parser, Token *token, int level);
	bool isInterval(Parser &parser, Token *token, int level);
	bool isCountIntervalBegin(Parser &parser, Token *token, int level);
	bool isCountIntervalEnd(Parser &parser, Token *token, int level);
	bool isCountIntervalSeparator(Parser &parser, Token *token, int level);
	bool isCountInterval(Parser &parser, Token *token, int level);
	bool isGroupBegin(Parser &parser, Token *token, int level);
	bool isGroupEnd(Parser &parser, Token *token, int level);
	bool isGroup(Parser &parser, Token *token, int level);
	bool isExpressionTerm(Parser &parser, Token *token, int level);
	bool isExpressionTermExtended(Parser &parser, Token *token, int level);
	bool isExpression(Parser &parser, Token *token, int level);
	bool isAssign(Parser &parser, Token *token, int level);
	bool isStatementEnd(Parser &parser, Token *token, int level);
	bool isWS(Parser &parser, Token *token, int level);
	bool isStatement(Parser &parser, Token *token, int level);

};

#endif

