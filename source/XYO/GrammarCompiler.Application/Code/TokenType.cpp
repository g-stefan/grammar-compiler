// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/TokenType.hpp>

namespace XYO::GrammarCompiler::Application::Code {

#define TOKENTYPE_STR_B(X) #X
#define TOKENTYPE_STR_A(X) TOKENTYPE_STR_B(X)
#define TOKENTYPE_STR(name)                   \
	case TokenType::name:                 \
		return TOKENTYPE_STR_A(name); \
		break

	const char *getTokenTypeString(Token::Type type) {
		switch (type) {
			TOKENTYPE_STR(Unknown);
			TOKENTYPE_STR(BOM);
			TOKENTYPE_STR(NewLine);
			TOKENTYPE_STR(WhiteSpaceContent);
			TOKENTYPE_STR(WhiteSpace);
			TOKENTYPE_STR(LineComment);
			TOKENTYPE_STR(MultilineCommentContent);
			TOKENTYPE_STR(MultilineComment);
			TOKENTYPE_STR(WhiteSpaceOrComment);
			TOKENTYPE_STR(Comment);
			TOKENTYPE_STR(Number);
			TOKENTYPE_STR(String);
			TOKENTYPE_STR(Symbol);
			TOKENTYPE_STR(Any);
			TOKENTYPE_STR(GroupBegin);
			TOKENTYPE_STR(GroupEnd);
			TOKENTYPE_STR(Group);
			TOKENTYPE_STR(IntervalBegin);
			TOKENTYPE_STR(IntervalEnd);
			TOKENTYPE_STR(IntervalSeparator);
			TOKENTYPE_STR(Interval);
			TOKENTYPE_STR(CountIntervalBegin);
			TOKENTYPE_STR(CountIntervalEnd);
			TOKENTYPE_STR(CountIntervalSeparator);
			TOKENTYPE_STR(CountInterval);
			TOKENTYPE_STR(ExpressionTerm);
			TOKENTYPE_STR(ExpressionTermExtended);
			TOKENTYPE_STR(ExpressionOperator);
			TOKENTYPE_STR(ExpressionPrecedence);
			TOKENTYPE_STR(ExpressionEnd);
			TOKENTYPE_STR(Alternation);
			TOKENTYPE_STR(Concatenation);
			TOKENTYPE_STR(Negation);
			TOKENTYPE_STR(ZeroOrOne);
			TOKENTYPE_STR(ZeroOrMore);
			TOKENTYPE_STR(OneOrMore);
			TOKENTYPE_STR(Expression);
			TOKENTYPE_STR(Assign);
			TOKENTYPE_STR(Statement);
			TOKENTYPE_STR(StatementEnd);
			// ---
			TOKENTYPE_STR(WS);
		default:
			break;
		};

		return "Unknown";
	};

};
