// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler/Parser.hpp>

namespace XYO::GrammarCompiler {

	ParserMemoization::ParserMemoization() {
		table = nullptr;
		capacity = 0;
		count = 0;
	};

	ParserMemoization::~ParserMemoization() {
		delete[] table;
	};

	bool ParserMemoization::has(size_t position, Token::Type type) {
		if (count == 0) {
			return false;
		};
		uint64_t value = key(position, type);
		size_t mask = capacity - 1;
		size_t index;
		for (index = hash(value, mask); table[index] != 0; index = (index + 1) & mask) {
			if (table[index] == value) {
				return true;
			};
		};
		return false;
	};

	void ParserMemoization::set(size_t position, Token::Type type) {
		if ((count + 1) * 2 > capacity) {
			grow();
		};
		uint64_t value = key(position, type);
		size_t mask = capacity - 1;
		size_t index;
		for (index = hash(value, mask); table[index] != 0; index = (index + 1) & mask) {
			if (table[index] == value) {
				return;
			};
		};
		table[index] = value;
		++count;
	};

	void ParserMemoization::empty() {
		delete[] table;
		table = nullptr;
		capacity = 0;
		count = 0;
	};

	void ParserMemoization::grow() {
		uint64_t *oldTable = table;
		size_t oldCapacity = capacity;

		capacity = (capacity == 0) ? 1024 : capacity * 2;
		table = new uint64_t[capacity]();

		size_t mask = capacity - 1;
		size_t k;
		size_t index;
		for (k = 0; k < oldCapacity; ++k) {
			if (oldTable[k] == 0) {
				continue;
			};
			for (index = hash(oldTable[k], mask); table[index] != 0; index = (index + 1) & mask) {
			};
			table[index] = oldTable[k];
		};

		delete[] oldTable;
	};

	Parser::Parser() {
		tokenList = TokenList::newNode();
	};

	Parser::~Parser() {
		TokenList::destructor(tokenList);
	};

	bool Parser::parse(ParserProcessToken *parseTable) {
		if (input.isEof()) {
			return false;
		};

		while (!input.isEof()) {
			size_t position = input.position;
			Token *tail = tokenList->childTail;
			ParserProcessToken *processToken;
			for (processToken = parseTable; *processToken != nullptr; ++processToken) {
				if ((*processToken)(*this, tokenList, 1024)) {
					if (input.position != position) {
						break;
					};
					// matched without consuming input, discard and try next
					while (tokenList->childTail != tail) {
						Token *node = tokenList->childTail;
						TokenList::extract(tokenList, node);
						TokenList::destructor(node);
					};
				};
			};
			if (*processToken == nullptr) {
				break;
			};
		};

		return input.isEof();
	};

	bool Parser::parseContent(const String &content, ParserProcessToken *parseTable) {
		memoization.empty();
		input.setContent(content);
		return parse(parseTable);
	};

	bool Parser::parseFile(const char *fileName, ParserProcessToken *parseTable) {
		String content;
		if (!Shell::fileGetContents(fileName, content)) {
			return false;
		};
		return parseContent(content, parseTable);
	};

	bool Parser::inputIs(const char *value) {
		size_t length = strlen(value);
		if (input.size - input.position < length) {
			return false;
		};
		if (memcmp(input.buffer + input.position, value, length) != 0) {
			return false;
		};
		input.skip(length);
		return true;
	};

	bool Parser::inputIsToken(const char *value, Token::Type type, Token *token) {
		if (inputIs(value)) {
			tokenAddValue(token, type, value, strlen(value));
			return true;
		};
		return false;
	};

	bool Parser::inputIsZeroOrMoreTokenValue(const char *value, Token::Type type, Token *token) {
		if (*value == 0) {
			return true;
		};
		while (inputIs(value)) {
			token->value += value;
		};
		return true;
	};

	bool Parser::inputIsOneOrMoreToken(const char *value, Token::Type type, Token *token) {
		if (inputIs(value)) {
			Token *node = TokenList::newNode();
			node->type = type;
			node->value = value;
			TokenList::pushToTail(token, node);
			return inputIsZeroOrMoreTokenValue(value, type, node);
		};
		return false;
	};

	bool Parser::inputIsTokenThenSet(ParserProcessToken parserProcessToken, Token::Type type, Token *token, int level) {
		size_t position = input.position;
		Token *node = TokenList::newNode();
		node->type = type;
		if (parserProcessToken(*this, node, level)) {
			TokenList::pushToTail(token, node);
			return true;
		};
		input.setPosition(position);
		TokenList::destructor(node);
		return false;
	};

	// Each input byte must be between the corresponding bytes of a and b.
	bool Parser::inputIsBetween(const char *a, const char *b, Token::Type type, Token *token) {
		size_t length = strlen(a);
		if ((length == 0) || (strlen(b) != length)) {
			return false;
		};
		if (input.size - input.position < length) {
			return false;
		};
		const uint8_t *value = reinterpret_cast<const uint8_t *>(input.buffer + input.position);
		size_t k;
		for (k = 0; k < length; ++k) {
			if ((value[k] < static_cast<uint8_t>(a[k])) || (value[k] > static_cast<uint8_t>(b[k]))) {
				return false;
			};
		};
		tokenAddValue(token, type, input.buffer + input.position, length);
		input.skip(length);
		return true;
	};

	bool Parser::inputIsTokenAny(Token::Type type, Token *token) {
		if (input.isEof()) {
			return false;
		};
		tokenAddValue(token, type, input.buffer + input.position, 1);
		input.skip(1);
		return true;
	};

	// Same result as tokenChildToValue, without allocating a node for each consecutive value.
	void Parser::tokenAddValue(Token *token, Token::Type type, const char *value, size_t length) {
		Token *tail = token->childTail;
		if (tail != nullptr) {
			if ((tail->type == type) && (type == token->type) && (tail->childHead == nullptr)) {
				tail->value.concatenate(value, length);
				return;
			};
		};
		Token *node = TokenList::newNode();
		node->type = type;
		node->value.set(value, length);
		TokenList::pushToTail(token, node);
	};

	// Merge consecutive leaf children of the same type as the token,
	// if only one remains, move its value to the token.
	void Parser::tokenChildToValue(Token *token) {
		Token *scan;
		Token *next;
		Token *index = nullptr;

		for (scan = token->childHead; scan != nullptr; scan = next) {
			next = scan->next;
			if ((scan->type != token->type) || (scan->childHead != nullptr)) {
				index = nullptr;
				continue;
			};
			if (index == nullptr) {
				index = scan;
				continue;
			};
			index->value += scan->value;
			TokenList::extract(token, scan);
			TokenList::destructor(scan);
		};

		scan = token->childHead;
		if (scan == nullptr) {
			return;
		};
		if (scan->next != nullptr) {
			return;
		};
		if ((scan->type == token->type) && (scan->childHead == nullptr)) {
			token->value += scan->value;
			TokenList::extract(token, scan);
			TokenList::destructor(scan);
		};
	};

	bool Parser::codeBegin(Token *&node, Token::Type type, int &level, size_t &position) {
		position = input.position;
		if (memoization.has(position, type)) {
			return false;
		};

		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		node = TokenList::newNode();
		node->type = type;

		return true;
	};

	void Parser::codeEndTrue(Token *token, Token *node) {
		tokenChildToValue(node);
		TokenList::pushToTail(token, node);
	};

	void Parser::codeEndFalse(Token *node, Token::Type type, size_t position) {
		input.setPosition(position);
		TokenList::destructor(node);
		memoization.set(position, type);
	};

};
