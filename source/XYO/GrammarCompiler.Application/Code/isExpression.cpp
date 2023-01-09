// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	bool isGroup(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Group, level)) {
			return false;
		};

		if (parser.inputIsToken("(", TokenType::GroupBegin, node)) {

			isWhiteSpaceOrComment(parser, node, level);
			isExpression(parser, node, level);
			isWhiteSpaceOrComment(parser, node, level);

			if (parser.inputIsToken(")", TokenType::GroupEnd, node)) {
				TokenList::pushToTail(token, node);
				return true;
			};
		};

		parser.codeEndFalse(node, TokenType::Group);
		return false;
	};

	bool isInterval(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Interval, level)) {
			return false;
		};

		if (parser.inputIsToken("[", TokenType::IntervalBegin, node)) {

			isWhiteSpaceOrComment(parser, node, level);

			if (isString(parser, node, level)) {

				isWhiteSpaceOrComment(parser, node, level);

				if (parser.inputIsToken(",", TokenType::IntervalSeparator, node)) {

					isWhiteSpaceOrComment(parser, node, level);

					if (isString(parser, node, level)) {

						isWhiteSpaceOrComment(parser, node, level);

						if (parser.inputIsToken("]", TokenType::IntervalEnd, node)) {
							TokenList::pushToTail(token, node);
							return true;
						};
					};
				};
			};
		};

		parser.codeEndFalse(node, TokenType::Interval);
		return false;
	};

	bool isCountInterval(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::CountInterval, level)) {
			return false;
		};

		if (parser.inputIsToken("{", TokenType::CountIntervalBegin, node)) {

			isWhiteSpaceOrComment(parser, node, level);

			if (isNumber(parser, node, level)) {

				isWhiteSpaceOrComment(parser, node, level);

				if (parser.inputIsToken("}", TokenType::CountIntervalEnd, node)) {
					TokenList::pushToTail(token, node);
					return true;
				};

				if (parser.inputIsToken(",", TokenType::CountIntervalSeparator, node)) {

					isWhiteSpaceOrComment(parser, node, level);

					if (isNumber(parser, node, level)) {

						isWhiteSpaceOrComment(parser, node, level);

						if (parser.inputIsToken("}", TokenType::CountIntervalEnd, node)) {
							TokenList::pushToTail(token, node);
							return true;
						};
					};
				};
			};
		};

		parser.codeEndFalse(node, TokenType::CountInterval);
		return false;
	};

	bool isExpressionUnaryLeft(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Negation, level)) {
			return false;
		};

		node->type = TokenType::ExpressionTerm;

		if (parser.inputIsToken("!", TokenType::Negation, node)) {

			isWhiteSpaceOrComment(parser, node, level);

			if (isExpressionTerm(parser, node, level)) {
				TokenList::pushToTail(token, node);
				return true;
			};
		};

		parser.codeEndFalse(node, TokenType::Negation);
		return false;
	};

	bool isExpressionTerm(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::ExpressionTerm, level)) {
			return false;
		};

		if (isExpressionUnaryLeft(parser, node, level)) {

			isWhiteSpaceOrComment(parser, node, level);

			if (parser.inputIsToken("?", TokenType::ZeroOrOne, node) || parser.inputIsToken("*", TokenType::ZeroOrMore, node) || parser.inputIsToken("+", TokenType::OneOrMore, node) || isCountInterval(parser, node, level)) {
				TokenList::pushToTail(token, node);
				return true;
			};

			TokenList::pushToTail(token, node);
			return true;
		};

		if (isString(parser, node, level) || isSymbol(parser, node, level) || isAny(parser, node, level) || isGroup(parser, node, level) || isInterval(parser, node, level)) {

			isWhiteSpaceOrComment(parser, node, level);

			if (parser.inputIsToken("?", TokenType::ZeroOrOne, node) || parser.inputIsToken("*", TokenType::ZeroOrMore, node) || parser.inputIsToken("+", TokenType::OneOrMore, node) || isCountInterval(parser, node, level)) {
				TokenList::pushToTail(token, node);
				return true;
			};

			TokenList::pushToTail(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::ExpressionTerm);
		return false;
	};

	bool isExpressionOperator(Parser &parser, Token *token, int level) {
		--level;
		if (level <= 0) {
			throw std::runtime_error("parser-stack-overflow");
		};

		// ---

		if (parser.inputIsToken("|", TokenType::Alternation, token)) {
			return true;
		};

		if (parser.inputIsToken("&", TokenType::Concatenation, token)) {
			return true;
		};

		return false;
	};

	bool isExpression(Parser &parser, Token *token, int level) {
		Token *node;
		if (!parser.codeBegin(node, TokenType::Expression, level)) {
			return false;
		};

		// ---

		bool hasExpression = false;

		while (isExpressionTerm(parser, node, level)) {

			hasExpression = true;

			isWhiteSpaceOrComment(parser, node, level);

			if (isExpressionOperator(parser, node, level)) {

				isWhiteSpaceOrComment(parser, node, level);

				continue;
			};

			break;
		};

		if (hasExpression) {
			TokenList::pushToTail(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Expression);
		return false;
	};

};
