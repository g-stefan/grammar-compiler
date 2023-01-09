// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_PARSER_HPP
#define XYO_GRAMMARCOMPILER_PARSER_HPP

#ifndef XYO_GRAMMARCOMPILER_INPUT_HPP
#	include <XYO/GrammarCompiler/Input.hpp>
#endif

#ifndef XYO_GRAMMARCOMPILER_TOKEN_HPP
#	include <XYO/GrammarCompiler/Token.hpp>
#endif

namespace XYO::GrammarCompiler {

	class Parser;

	typedef bool (*ParserProcessToken)(Parser &parser, Token *token, int level);

	struct ParserMemoization {
			size_t index;
			Token::Type type;

			inline bool operator<(const ParserMemoization &r) const {
				if (index < r.index) {
					return true;
				};
				if (type < r.type) {
					return true;
				};
				return false;
			};

			inline bool operator==(const ParserMemoization &r) const {
				if (index == r.index) {
					if (type == r.type) {
						return true;
					};
				};
				return false;
			};
	};

	class Parser {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(Parser);

		public:
			Token *tokenList;
			Input input;

			TRedBlackTree<ParserMemoization, bool> memoization;

		public:
			XYO_GRAMMARCOMPILER_EXPORT Parser(size_t inputStackSize_ = 16384);
			XYO_GRAMMARCOMPILER_EXPORT ~Parser();

			XYO_GRAMMARCOMPILER_EXPORT bool parse(ParserProcessToken *parseTable);
			XYO_GRAMMARCOMPILER_EXPORT bool parseFile(const char *fileName, ParserProcessToken *parseTable);

			XYO_GRAMMARCOMPILER_EXPORT bool inputIs(const char *value);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsLessThanOrEqualTo(const char *value, String &out);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsGreaterThanOrEqualTo(const char *value, String &out);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsToken(const char *value, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsZeroOrMoreTokenValue(const char *value, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsOneOrMoreToken(const char *value, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT void inputRestore(const char *value);
			XYO_GRAMMARCOMPILER_EXPORT void inputRestore(Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsTokenThenSet(ParserProcessToken parserProcessToken, Token::Type type, Token *token, int level);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsBetween(const char *a, const char *b, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsTokenAny(Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT static void tokenChildToValue(Token *token);

			static inline void initMemory() {
				TMemory<TRedBlackTree<ParserMemoization, bool>>::initMemory();
				TMemory<Token>::initMemory();
			};

			XYO_GRAMMARCOMPILER_EXPORT bool codeBegin(Token *&node, Token::Type type, int &level);
			XYO_GRAMMARCOMPILER_EXPORT void codeEndTrue(Token *token, Token *node);
			XYO_GRAMMARCOMPILER_EXPORT void codeEndFalse(Token *node, Token::Type type);

			XYO_GRAMMARCOMPILER_EXPORT bool memoizationIsFalse(Token::Type type);
			XYO_GRAMMARCOMPILER_EXPORT void memoizationSetFalse(Token::Type type);
	};

};

#endif
