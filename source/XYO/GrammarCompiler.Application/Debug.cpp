// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Debug.hpp>
#include <XYO/GrammarCompiler.Application/Code/TokenType.hpp>

namespace XYO::GrammarCompiler::Application::Debug {

	using namespace XYO::GrammarCompiler::Application::Code;

	void inputPrint(Input &input) {
		printf("Position: %zu / %zu, line %zu\r\n", input.position, input.size, input.getLineNumber(input.position));
		printf("Hex: %02X\r\n", static_cast<uint8_t>(input.input));
		printf("CHR: %c\r\n", input.input);
	};

	void tokenPrintType(Token *token) {
		const char *tokenName = Code::getTokenTypeString(token->type);
		printf("%s", tokenName);
	};

	void tokenPrint(Token *token, Token *parentScan) {
		Token *scan;
		fflush(stdout);
		for (scan = token->parent; (scan != parentScan) && (scan != nullptr); scan = scan->parent) {
			printf("    ");
		};
		if (token->parent) {
			printf("+-  ");
		};
		tokenPrintType(token);
		printf(" [%s]", token->value.value());
		printf("\n");
		fflush(stdout);
	};

	void tokenPrintList(Token *tokenList, Token *parentScan) {
		Token *scan;

		scan = tokenList;

		if (!parentScan) {
			scan = TokenList::begin(tokenList);
		};

		for (; scan != nullptr;) {
			Debug::tokenPrint(scan);
			scan = TokenList::successor(scan);
			if (scan == parentScan) {
				break;
			};
		};
	};
};
