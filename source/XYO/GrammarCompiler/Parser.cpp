// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler/Parser.hpp>

namespace XYO::GrammarCompiler {

	Parser::Parser(size_t inputStackSize_) : input(inputStackSize_) {
		tokenList = TokenList::newNode();
	};

	Parser::~Parser() {
		TokenList::destructor(tokenList);
	};

	bool Parser::parse(ParserProcessToken *parseTable) {

		if (input.isEof()) {
			return false;
		};

		if (!input.read()) {
			return false;
		};

		while (!input.isEof()) {
			ParserProcessToken *processToken;
			for (processToken = parseTable; *processToken != nullptr; ++processToken) {
				if ((*processToken)(*this, tokenList, 1024)) {
					break;
				};
			};
			if (*processToken == nullptr) {
				break;
			};
		};

		return input.isEof();
	};

	bool Parser::parseFile(const char *fileName, ParserProcessToken *parseTable) {
		File file;

		input.setIRead(&file);
		if (file.openRead(fileName)) {
			if (parse(parseTable)) {
				input.setIRead(nullptr);
				file.close();
				return true;
			};
			file.close();
		};

		input.setIRead(nullptr);
		return false;
	};

	bool Parser::inputIs(const char *value) {
		int k;
		for (k = 0; value[k] != 0; ++k) {
			if (input == value[k]) {
				if (input.read()) {
					continue;
				};
				++k;
			};
			break;
		};
		if (value[k] == 0) {
			return true;
		};
		for (--k; k >= 0; --k) {
			input.push();
			input = value[k];
		};
		return false;
	};

	bool Parser::inputIsLessThanOrEqualTo(const char *value, String &out) {
		out = "";
		int k;
		for (k = 0; value[k] != 0; ++k) {
			if (input <= value[k]) {
				out += input;
				if (input.read()) {
					continue;
				};
				++k;
			};
			break;
		};
		if (value[k] == 0) {
			return true;
		};
		for (--k; k >= 0; --k) {
			input.push();
			input = value[k];
		};
		return false;
	};

	bool Parser::inputIsGreaterThanOrEqualTo(const char *value, String &out) {
		out = "";
		int k;
		for (k = 0; value[k] != 0; ++k) {
			if (input >= value[k]) {
				out += input;
				if (input.read()) {
					continue;
				};
				++k;
			};
			break;
		};
		if (value[k] == 0) {
			return true;
		};
		for (--k; k >= 0; --k) {
			input.push();
			input = value[k];
		};
		return false;
	};

	bool Parser::inputIsToken(const char *value, Token::Type type, Token *token) {
		if (inputIs(value)) {
			Token *node = TokenList::newNode();
			node->type = type;
			node->value = value;
			TokenList::pushToTail(token, node);
			return true;
		};
		return false;
	};

	bool Parser::inputIsZeroOrMoreTokenValue(const char *value, Token::Type type, Token *token) {
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

	void Parser::inputRestore(const char *value) {
		int k = strlen(value);
		for (--k; k >= 0; --k) {
			input.push();
			input = value[k];
		};
	};

	void Parser::inputRestore(Token *token) {
		Token *scan;
		for (scan = TokenList::end(token); scan != nullptr; scan = TokenList::predecessor(scan)) {
			inputRestore(scan->value.value());
		};
	};

	bool Parser::inputIsTokenThenSet(ParserProcessToken parserProcessToken, Token::Type type, Token *token, int level) {
		Token *node = TokenList::newNode();
		node->type = type;
		if (parserProcessToken(*this, node, level)) {
			TokenList::pushToTail(token, node);
			return true;
		};
		inputRestore(node);
		TokenList::destructor(node);
		return false;
	};

	bool Parser::inputIsBetween(const char *a, const char *b, Token::Type type, Token *token) {
		String value;
		if (inputIsGreaterThanOrEqualTo(a, value)) {
			if (value <= b) {
				Token *node = TokenList::newNode();
				node->type = type;
				node->value = value;
				TokenList::pushToTail(token, node);
				return true;
			};
			inputRestore(value.value());
		};
		return false;
	};

	bool Parser::inputIsTokenAny(Token::Type type, Token *token) {
		if (input.isEof()) {
			return false;
		};
		Token *node = TokenList::newNode();
		node->type = type;
		char buf[2];
		buf[0] = input;
		buf[1] = 0;
		node->value = buf;
		TokenList::pushToTail(token, node);

		input.read();
		return true;
	};

	void Parser::tokenChildToValue(Token *token) {
		Token *scan;
		Token *node;
		Token *index;

		for (index = token->childHead; index != nullptr; index = index->next) {
			if (index->type == token->type) {
				break;
			};
		};
		if (index == nullptr) {
			return;
		};

		scan = index->next;
		while (scan != nullptr) {
			if (scan->type == index->type) {
				index->value += scan->value;
				node = scan;
				scan = scan->next;
				TokenList::extract(node->parent, node);
				TokenList::destructor(node);
				continue;
			};
			for (index = scan; index != nullptr; index = index->next) {
				if (index->type == token->type) {
					break;
				};
			};
			if (index == nullptr) {
				return;
			};
			scan = index->next;
		};

		if (token->childHead->next == nullptr) {
			node = token->childHead;
			if (node->type == token->type) {
				token->value += node->value;
				TokenList::extract(node->parent, node);
				TokenList::destructor(node);
			};
		};
	};

	bool Parser::codeBegin(Token *&node, Token::Type type, int &level) {
		if (memoizationIsFalse(type)) {
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

	void Parser::codeEndFalse(Token *node, Token::Type type) {
		inputRestore(node);
		TokenList::destructor(node);
		memoizationSetFalse(type);
	};

	bool Parser::memoizationIsFalse(Token::Type type) {
		TRedBlackTree<ParserMemoization, bool>::Node *node;
		ParserMemoization info;

		info.index = input.fileIndex;
		info.type = type;

		node = memoization.find(info);
		if (node) {
			if (!node->value) {
				return true;
			};
		};

		return false;
	};

	void Parser::memoizationSetFalse(Token::Type type) {
		TRedBlackTree<ParserMemoization, bool>::Node *node;
		ParserMemoization info;

		info.index = input.fileIndex;
		info.type = type;

		node = memoization.find(info);
		if (node) {
			node->value = false;
			return;
		};

		memoization.insert(info, false);
	};
};
