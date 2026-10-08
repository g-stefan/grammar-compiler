// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
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

	// Set of (position, type) pairs where a rule is known to fail, open addressing hash set.
	class ParserMemoization {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(ParserMemoization);

		public:
			uint64_t *table;
			size_t capacity;
			size_t count;

			XYO_GRAMMARCOMPILER_EXPORT ParserMemoization();
			XYO_GRAMMARCOMPILER_EXPORT ~ParserMemoization();

			XYO_GRAMMARCOMPILER_EXPORT bool has(size_t position, Token::Type type);
			XYO_GRAMMARCOMPILER_EXPORT void set(size_t position, Token::Type type);
			XYO_GRAMMARCOMPILER_EXPORT void empty();

		protected:
			// never 0, 0 marks an empty slot
			static inline uint64_t key(size_t position, Token::Type type) {
				return ((static_cast<uint64_t>(position) + 1) << 16) | type;
			};

			static inline size_t hash(uint64_t key, size_t mask) {
				return static_cast<size_t>((key * 0x9E3779B97F4A7C15ULL) >> 32) & mask;
			};

			void grow();
	};

	class Parser {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Parser);

		public:
			Token *tokenList;
			Input input;

			ParserMemoization memoization;

		public:
			XYO_GRAMMARCOMPILER_EXPORT Parser();
			XYO_GRAMMARCOMPILER_EXPORT ~Parser();

			XYO_GRAMMARCOMPILER_EXPORT bool parse(ParserProcessToken *parseTable);
			XYO_GRAMMARCOMPILER_EXPORT bool parseContent(const String &content, ParserProcessToken *parseTable);
			XYO_GRAMMARCOMPILER_EXPORT bool parseFile(const char *fileName, ParserProcessToken *parseTable);

			inline size_t inputPosition() {
				return input.position;
			};

			inline void inputRestore(size_t position) {
				input.setPosition(position);
			};

			XYO_GRAMMARCOMPILER_EXPORT bool inputIs(const char *value);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsToken(const char *value, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsZeroOrMoreTokenValue(const char *value, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsOneOrMoreToken(const char *value, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsTokenThenSet(ParserProcessToken parserProcessToken, Token::Type type, Token *token, int level);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsBetween(const char *a, const char *b, Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT bool inputIsTokenAny(Token::Type type, Token *token);
			XYO_GRAMMARCOMPILER_EXPORT static void tokenAddValue(Token *token, Token::Type type, const char *value, size_t length);
			XYO_GRAMMARCOMPILER_EXPORT static void tokenChildToValue(Token *token);

			// Move children of node to token, node can have no children
			static inline void tokenTransferChild(Token *token, Token *node) {
				if (node->childHead == nullptr) {
					return;
				};
				TokenList::transferChild(token, node);
			};

			static inline void initMemory() {
				TMemory<Token>::initMemory();
			};

			XYO_GRAMMARCOMPILER_EXPORT bool codeBegin(Token *&node, Token::Type type, int &level, size_t &position);
			XYO_GRAMMARCOMPILER_EXPORT void codeEndTrue(Token *token, Token *node);
			XYO_GRAMMARCOMPILER_EXPORT void codeEndFalse(Token *node, Token::Type type, size_t position);

			inline size_t getErrorLine() {
				return input.getLineNumber(input.farthestPosition);
			};
	};

};

#endif
