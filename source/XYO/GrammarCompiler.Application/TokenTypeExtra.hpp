// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_TOKENTYPEX_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_TOKENTYPEX_HPP

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_CODE_TOKENTYPE_HPP
#	include <XYO/GrammarCompiler.Application/Code/TokenType.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Code {

	namespace TokenType {
		enum : Token::Type {
			ExpressionOperator = 1000,
			ExpressionPrecedence
		};
	};

	const char *getTokenTypeExtraString(Token::Type type);

	bool findByTypeNext(Token *this_, Token::Type type, Token *&token);
	bool findByTypeNext(Token *this_, Token::Type type1, Token::Type type2, Token *&token);
};

#endif
