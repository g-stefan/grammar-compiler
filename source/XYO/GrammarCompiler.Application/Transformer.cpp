// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Transformer.hpp>
#include <XYO/GrammarCompiler.Application/Code/Code.hpp>
#include <XYO/GrammarCompiler.Application/TokenTypeExtra.hpp>

namespace XYO::GrammarCompiler::Application::Transformer {

	using namespace XYO::GrammarCompiler::Application::Code;

	struct ExpressionOperatorPrecedence {
			int level;
			Token::Type subType;
	};

	ExpressionOperatorPrecedence expressionOperatorPrecedenceTable[] = {
	    {0, TokenType::Alternation},
	    {1, TokenType::Concatenation}};

	int expressionOperatorPrecedenceGetLevel(Token *token) {
		int index;
		int expressionOperatorPrecedenceLn = sizeof(expressionOperatorPrecedenceTable) / sizeof(ExpressionOperatorPrecedence);
		for (index = 0; index < expressionOperatorPrecedenceLn; ++index) {
			if (expressionOperatorPrecedenceTable[index].subType == token->subType) {
				return expressionOperatorPrecedenceTable[index].level;
			};
		};
		return -1;
	};

	bool expressionOperatorPrecedenceScan(Token *termBegin, int level, Token *&termEnd, int &nextLevel) {
		Token *eTerm = nullptr;
		Token *eOperator = nullptr;
		Token *index = termBegin;
		bool found = false;
		while (findByTypeNext(index, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, eTerm)) {
			if (findByTypeNext(eTerm, TokenType::ExpressionOperator, eOperator)) {
				nextLevel = expressionOperatorPrecedenceGetLevel(eOperator);
				if (nextLevel == level) {
					found = true;
					index = eOperator->next;
					continue;
				};
			};
			break;
		};
		termEnd = eTerm;
		return found;
	};

	void expressionOperatorPrecedenceSubstitute(Token *termBegin, Token *termEnd, Token *&newTerm) {
		Token *scan;

		Token *parent = termBegin->parent;
		Token *anchor = termBegin->back;
		TokenList::extractList(parent, termBegin, termEnd);

		newTerm = TokenList::newNode();
		newTerm->type = TokenType::ExpressionPrecedence;
		TokenList::addListToTail(newTerm, termBegin, termEnd);
		TokenList::insertNode(parent, anchor, newTerm);
	};

	void expressionOperatorPrecedenceLevel(Token *token, int lastLevel, int level) {
		Token *termBegin = token;
		Token *termEnd = token;
		int nextLevel;

		if (expressionOperatorPrecedenceScan(termBegin, level, termEnd, nextLevel)) {
			Token *newTerm = termEnd;
			if (lastLevel < level || nextLevel < level) {
				expressionOperatorPrecedenceSubstitute(termBegin, termEnd, newTerm);
			};
			expressionOperatorPrecedenceLevel(newTerm, level, nextLevel);
		} else {
			if (lastLevel > level) {
				Token *eTerm;
				Token *eOperator;
				if (findByTypeNext(token, TokenType::ExpressionOperator, eOperator)) {
					if (findByTypeNext(eOperator, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, eTerm)) {
						if (findByTypeNext(eTerm, TokenType::ExpressionOperator, eOperator)) {
							nextLevel = expressionOperatorPrecedenceGetLevel(eOperator);
							expressionOperatorPrecedenceLevel(eTerm, level, nextLevel);
						};
					};
				};
			};
		};
	};

	void expressionOperatorPrecedenceSubType(Token *&token) {
		Token *node;
		for (node = TokenList::begin(token); node != nullptr; node = TokenList::successor(node)) {
			int index;
			int expressionOperatorPrecedenceLn = sizeof(expressionOperatorPrecedenceTable) / sizeof(ExpressionOperatorPrecedence);
			for (index = 0; index < expressionOperatorPrecedenceLn; ++index) {
				if (expressionOperatorPrecedenceTable[index].subType == token->type) {
					token->subType = token->type;
					token->type = TokenType::ExpressionOperator;
					break;
				};
			};
		};
	};

	void expressionOperatorPrecedence(Token *&token) {
		expressionOperatorPrecedenceSubType(token);

		Token *eOperator;
		if (findByTypeNext(token->childHead, TokenType::ExpressionOperator, eOperator)) {
			int level = expressionOperatorPrecedenceGetLevel(eOperator);
			expressionOperatorPrecedenceLevel(token->childHead, level, level);
		};
	};

	void removeNoise(Token *&token) {
		Token *scan;
		Token *node;

		for (scan = TokenList::begin(token); scan != nullptr;) {
			node = scan;
			// clang-format off
			if ((node->type == TokenType::WhiteSpace)
			|| (node->type == TokenType::Comment)
			|| (node->type == TokenType::BOM)
			|| (node->type == TokenType::WhiteSpaceOrComment)
			|| (node->type == TokenType::WS)
			|| (node->type == TokenType::StatementEnd)
			|| (node->type == TokenType::GroupBegin)
			|| (node->type == TokenType::GroupEnd)
			|| (node->type == TokenType::IntervalBegin)
			|| (node->type == TokenType::IntervalSeparator)
			|| (node->type == TokenType::IntervalEnd)
			|| (node->type == TokenType::CountIntervalBegin)
			|| (node->type == TokenType::CountIntervalSeparator)
			|| (node->type == TokenType::CountIntervalEnd)) {
				// clang-format on
				scan = TokenList::successorNoChild(scan);
				TokenList::extract(node->parent, node);
				TokenList::destructor(node);
				continue;
			};
			scan = TokenList::successor(scan);
		};
	};

	void transform(Token *&token) {
		Token *scan;
		Token *node;
		Token *parent;

		for (scan = token; scan != nullptr;) {
			if (scan->type == TokenType::ExpressionTermExtended) {
				if (scan->childHead) {
					if (scan->childTail == scan->childHead) {
						if (scan->childHead->type == TokenType::ExpressionTerm) {
							node = scan;

							parent = node->parent;
							node->parent = nullptr;
							node->type = TokenType::Unknown;
							transform(node->childHead);
							node->parent = parent;

							scan = TokenList::successorNoChild(scan);

							TokenList::transferChildAnchor(node->parent, node);
							TokenList::extract(node->parent, node);
							TokenList::destructor(node);
							continue;
						};
					};
				};
				scan->type = TokenType::ExpressionTerm;
				scan = TokenList::successor(scan);
				continue;
			};
			if (scan->type == TokenType::Alternation) {
				scan->type = TokenType::ExpressionOperator;
				scan->subType = TokenType::Alternation;
				scan = TokenList::successor(scan);
				continue;
			};
			if (scan->type == TokenType::Concatenation) {
				scan->type = TokenType::ExpressionOperator;
				scan->subType = TokenType::Concatenation;
				scan = TokenList::successor(scan);
				continue;
			};
			scan = TokenList::successor(scan);
		};
	};

	void orderOperations(Token *&token) {
		Token *scan;
		Token *node;
		Token *term;
		for (scan = TokenList::begin(token); scan != nullptr;) {
			node = scan;

			// clang-format off
			if ((node->type == TokenType::ZeroOrOne)
			|| (node->type == TokenType::ZeroOrMore)
			|| (node->type == TokenType::OneOrMore)
			|| (node->type == TokenType::CountInterval)) {
				// clang-format on
				scan = TokenList::successorNoChild(scan);

				if (node->back) {
					term = node->back;
					TokenList::extract(node->parent, node);
					TokenList::insertNode(term->parent, term->back, node);
					TokenList::extract(term->parent, term);
					TokenList::push(node, term);
				};

				continue;
			};

			scan = TokenList::successor(scan);
		};
	};

	void statementOperations(Token *&token) {
		Token *scan;
		Token *node;
		Token *term;

		node = TokenList::begin(token);
		if (node == nullptr) {
			return;
		};

		for (scan = node->childHead; scan != nullptr;) {
			node = scan;
			scan = scan->next;

			if (node->type == TokenType::Statement) {
				Token *Alternation;
				Token *Concatenation;
				Token *statementSymbol;
				Token *statementNext;
				Token *statementAssign;

				Alternation = nullptr;
				Concatenation = nullptr;
				statementNext = nullptr;
				statementAssign = nullptr;
				statementSymbol = nullptr;

				if (findByTypeNext(node->childHead, TokenType::Symbol, statementSymbol)) {

					findByTypeNext(statementSymbol->next, TokenType::Alternation, Alternation);
					findByTypeNext(statementSymbol->next, TokenType::Concatenation, Concatenation);
					if (Alternation || Concatenation) {

						if (Alternation) {
							statementNext = Alternation;
						};
						if (Concatenation) {
							statementNext = Concatenation;
						};

						if (findByTypeNext(statementNext->next, TokenType::Assign, statementAssign)) {

							//  find first statement node
							Token *index;
							Token *symbol;
							for (index = TokenList::begin(token)->childHead; index != nullptr; index = index->next) {
								if (index->type == TokenType::Statement) {
									if (findByTypeNext(index->childHead, TokenType::Symbol, symbol)) {
										if (symbol->value == statementSymbol->value) {
											break;
										};
									};
								};
							};

							if (index) {

								findByTypeNext(index->childHead, TokenType::Assign, index);
								index = index->next; // Assign / Expression / ...

								Token *listBeginNode;
								Token *listEndNode;
								listBeginNode = statementAssign->next;
								listEndNode = node->childTail;
								TokenList::extractList(node, listBeginNode, listEndNode);
								Token *listBegin = listBeginNode->childHead;
								Token *listEnd = listBeginNode->childTail;
								TokenList::extractList(listBeginNode, listBegin, listEnd);
								TokenList::destructor(listBeginNode);

								Token *iNode;

								iNode = TokenList::newNode();
								if (Alternation) {
									iNode->type = TokenType::ExpressionOperator;
									iNode->subType = TokenType::Alternation;
									iNode->value = "|";
								};
								if (Concatenation) {
									iNode->type = TokenType::ExpressionOperator;
									iNode->subType = TokenType::Concatenation;
									iNode->value = "&";
								};
								TokenList::pushToTail(index, iNode);

								TokenList::addListToTail(index, listBegin, listEnd);

								TokenList::extract(node->parent, node);
								TokenList::destructor(node);

								continue;
							};
						};
					};
				};
			};
		};
	};

	void statementExpressionPrecedence(Token *&token) {
		Token *scan;
		Token *node;
		Token *term;

		node = TokenList::begin(token);
		if (node == nullptr) {
			return;
		};

		for (scan = node->childHead; scan != nullptr;) {
			node = scan;
			scan = scan->next;
			if (node->type == TokenType::Statement) {
				Token *statementExpression = nullptr;
				if (findByTypeNext(node->childHead, TokenType::Expression, statementExpression)) {
					expressionOperatorPrecedence(statementExpression);
				};
			};
		};
	};

	void process(Token *&token) {
		removeNoise(token);
		transform(token);
		orderOperations(token);
		statementOperations(token);
		statementExpressionPrecedence(token);
	};

	size_t getLineCount(Token *&token) {
		size_t count = 1;
		Token *scan;
		for (scan = TokenList::begin(token); scan != nullptr; scan = TokenList::successor(scan)) {
			if (scan->type == TokenType::NewLine) {
				++count;
			};
		};
		return count;
	};
};
