// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Debug.hpp>
#include <XYO/GrammarCompiler.Application/Code/TokenType.hpp>
#include <XYO/GrammarCompiler.Application/TokenTypeExtra.hpp>

namespace XYO::GrammarCompiler::Application::Debug {

	using namespace XYO::GrammarCompiler::Application::Code;

	void inputPrint(Input &input) {
		printf("Hex: %02X", input.input);
		int index;
		for (index = 0; index < input.stackIndex; ++index) {
			printf("%02X", input.inputStack[index]);
		};
		printf("\r\n");

		printf("CHR: %c", input.input);

		for (index = 0; index < input.stackIndex; ++index) {
			printf("%c", input.inputStack[index]);
		};
		printf("\r\n");
	};

	void tokenPrintType(Token *token) {
		const char *tokenName = Code::getTokenTypeExtraString(token->type);
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
