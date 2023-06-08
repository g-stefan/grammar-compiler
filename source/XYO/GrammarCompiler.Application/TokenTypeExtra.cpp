// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/TokenTypeExtra.hpp>

namespace XYO::GrammarCompiler::Application::Code {

#define TOKENTYPEEXTRA_STR_B(X) #X
#define TOKENTYPEEXTRA_STR_A(X) TOKENTYPEEXTRA_STR_B(X)
#define TOKENTYPEEXTRA_STR(name)                   \
	case TokenType::name:                      \
		return TOKENTYPEEXTRA_STR_A(name); \
		break

	const char *getTokenTypeExtraString(Token::Type type) {
		switch (type) {
			TOKENTYPEEXTRA_STR(ExpressionOperator);
			TOKENTYPEEXTRA_STR(ExpressionPrecedence);
		default:
			break;
		};

		return getTokenTypeString(type);
	};

	bool findByTypeNext(Token *this_, Token::Type type, Token *&token) {
		for (token = this_; token != nullptr; token = token->next) {
			if (token->type == type) {
				return true;
			};
		};
		return false;
	};

	bool findByTypeNext(Token *this_, Token::Type type1, Token::Type type2, Token *&token) {
		for (token = this_; token != nullptr; token = token->next) {
			if (token->type == type1 || token->type == type2) {
				return true;
			};
		};
		return false;
	};

};
