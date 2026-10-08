// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/isBOM.cpp>
#include <XYO/GrammarCompiler.Application/Code/isNewLine.cpp>
#include <XYO/GrammarCompiler.Application/Code/isWhiteSpace.cpp>
#include <XYO/GrammarCompiler.Application/Code/isLineComment.cpp>
#include <XYO/GrammarCompiler.Application/Code/isMultilineComment.cpp>
#include <XYO/GrammarCompiler.Application/Code/isComment.cpp>
#include <XYO/GrammarCompiler.Application/Code/isWhiteSpaceOrComment.cpp>
#include <XYO/GrammarCompiler.Application/Code/isString.cpp>
#include <XYO/GrammarCompiler.Application/Code/isNumber.cpp>
#include <XYO/GrammarCompiler.Application/Code/isSymbol.cpp>
#include <XYO/GrammarCompiler.Application/Code/isAlternation.cpp>
#include <XYO/GrammarCompiler.Application/Code/isConcatenation.cpp>
#include <XYO/GrammarCompiler.Application/Code/isNegation.cpp>
#include <XYO/GrammarCompiler.Application/Code/isAny.cpp>
#include <XYO/GrammarCompiler.Application/Code/isZeroOrOne.cpp>
#include <XYO/GrammarCompiler.Application/Code/isZeroOrMore.cpp>
#include <XYO/GrammarCompiler.Application/Code/isOneOrMore.cpp>
#include <XYO/GrammarCompiler.Application/Code/isIntervalBegin.cpp>
#include <XYO/GrammarCompiler.Application/Code/isIntervalEnd.cpp>
#include <XYO/GrammarCompiler.Application/Code/isIntervalSeparator.cpp>
#include <XYO/GrammarCompiler.Application/Code/isInterval.cpp>
#include <XYO/GrammarCompiler.Application/Code/isCountIntervalBegin.cpp>
#include <XYO/GrammarCompiler.Application/Code/isCountIntervalEnd.cpp>
#include <XYO/GrammarCompiler.Application/Code/isCountIntervalSeparator.cpp>
#include <XYO/GrammarCompiler.Application/Code/isCountInterval.cpp>
#include <XYO/GrammarCompiler.Application/Code/isGroupBegin.cpp>
#include <XYO/GrammarCompiler.Application/Code/isGroupEnd.cpp>
#include <XYO/GrammarCompiler.Application/Code/isGroup.cpp>
#include <XYO/GrammarCompiler.Application/Code/isExpressionTerm.cpp>
#include <XYO/GrammarCompiler.Application/Code/isExpressionTermExtended.cpp>
#include <XYO/GrammarCompiler.Application/Code/isExpression.cpp>
#include <XYO/GrammarCompiler.Application/Code/isAssign.cpp>
#include <XYO/GrammarCompiler.Application/Code/isStatementEnd.cpp>
#include <XYO/GrammarCompiler.Application/Code/isWS.cpp>
#include <XYO/GrammarCompiler.Application/Code/isStatement.cpp>
#include <XYO/GrammarCompiler.Application/Code/ParserTable.cpp>
#include <XYO/GrammarCompiler.Application/Code/TokenType.cpp>

