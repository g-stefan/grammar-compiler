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

	namespace TokenType {
		enum : Token::Type {
			Unknown = 0,
			BOM,
			NewLine,
			WhiteSpaceContent,
			WhiteSpace,
			LineComment,
			LineCommentContent,
			MultilineCommentContent,
			MultilineComment,
			WhiteSpaceOrComment,
			Comment,
			Number,
			String,
			Symbol,
			Any,
			GroupBegin,
			GroupEnd,
			Group,
			IntervalBegin,
			IntervalEnd,
			IntervalSeparator,
			Interval,
			CountIntervalBegin,
			CountIntervalEnd,
			CountIntervalSeparator,
			CountInterval,
			ExpressionTerm,
			ExpressionTermExtended,
			ExpressionEnd,
			Alternation,
			Concatenation,
			Negation,
			ZeroOrOne,
			ZeroOrMore,
			OneOrMore,
			Expression,
			Assign,
			Statement,
			StatementEnd,
			WS
		};
	};

	const char *getTokenTypeString(Token::Type type);
};

#endif
