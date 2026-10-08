// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP

#ifndef XYO_GRAMMARCOMPILER_HPP
#	include <XYO/GrammarCompiler.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Code {
	using namespace XYO::GrammarCompiler;

	namespace TokenType {
		enum : Token::Type {
			Unknown = 0,
			BOM,
			NewLine,
			WhiteSpace,
			LineComment,
			MultilineComment,
			Comment,
			WhiteSpaceOrComment,
			String,
			Number,
			Symbol,
			Alternation,
			Concatenation,
			Negation,
			Any,
			ZeroOrOne,
			ZeroOrMore,
			OneOrMore,
			IntervalBegin,
			IntervalEnd,
			IntervalSeparator,
			Interval,
			CountIntervalBegin,
			CountIntervalEnd,
			CountIntervalSeparator,
			CountInterval,
			GroupBegin,
			GroupEnd,
			Group,
			ExpressionTerm,
			ExpressionTermExtended,
			Expression,
			Assign,
			StatementEnd,
			WS,
			Statement
		};
	};

	const char *getTokenTypeString(Token::Type type);
};

#endif

