// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP

#ifndef XYO_GRAMMARCOMPILER_TOKEN_HPP
#	include <XYO/GrammarCompiler/Token.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Code {

	struct TokenType {
			static const Token::Type Unknown = 0;
			static const Token::Type BOM = 1;
			static const Token::Type NewLine = 2;
			static const Token::Type WhiteSpaceContent = 3;
			static const Token::Type WhiteSpace = 4;
			static const Token::Type LineComment = 5;
			static const Token::Type MultilineCommentContent = 6;
			static const Token::Type MultilineComment = 7;
			static const Token::Type WhiteSpaceOrComment = 8;
			static const Token::Type Comment = 9;
			static const Token::Type Number = 10;
			static const Token::Type String = 11;
			static const Token::Type Symbol = 12;
			static const Token::Type Any = 13;
			static const Token::Type GroupBegin = 14;
			static const Token::Type GroupEnd = 15;
			static const Token::Type Group = 16;
			static const Token::Type IntervalBegin = 17;
			static const Token::Type IntervalEnd = 18;
			static const Token::Type IntervalSeparator = 19;
			static const Token::Type Interval = 20;
			static const Token::Type CountIntervalBegin = 21;
			static const Token::Type CountIntervalEnd = 22;
			static const Token::Type CountIntervalSeparator = 23;
			static const Token::Type CountInterval = 24;
			static const Token::Type ExpressionTerm = 25;
			static const Token::Type ExpressionTermExtended = 26;
			static const Token::Type ExpressionOperator = 27;
			static const Token::Type ExpressionPrecedence = 28;
			static const Token::Type ExpressionEnd = 29;
			static const Token::Type Alternation = 30;
			static const Token::Type Concatenation = 31;
			static const Token::Type Negation = 32;
			static const Token::Type ZeroOrOne = 33;
			static const Token::Type ZeroOrMore = 34;
			static const Token::Type OneOrMore = 35;
			static const Token::Type Expression = 36;
			static const Token::Type Assign = 37;
			static const Token::Type Statement = 38;
			static const Token::Type StatementEnd = 39;
			// ---
			static const Token::Type WS = 40;
	};

	const char *getTokenTypeString(Token::Type type);
};

#endif
