// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Processor.hpp>
#include <XYO/GrammarCompiler.Application/Code/Code.hpp>
#include <XYO/GrammarCompiler.Application/Debug.hpp>
#include <XYO/System.hpp>

namespace XYO::GrammarCompiler::Application {

	using namespace XYO::GrammarCompiler::Application::Code;

	Processor::Processor() {
		tokenList = nullptr;
	};

	Processor::~Processor(){};

	bool Processor::findByTypeNext(Token *this_, Token::Type type, Token *&token) {
		for (token = this_; token != nullptr; token = token->next) {
			if (token->type == type) {
				return true;
			};
		};
		return false;
	};

	bool Processor::findByTypeNext2(Token *this_, Token::Type type1, Token::Type type2, Token *&token) {
		for (token = this_; token != nullptr; token = token->next) {
			if (token->type == type1 || token->type == type2) {
				return true;
			};
		};
		return false;
	};

	struct ExpressionOperatorPrecedence {
			int level;
			Token::Type subType;
	};

	ExpressionOperatorPrecedence expressionOperatorPrecedenceTable[] = {
	    {0, TokenType::Alternation},
	    {1, TokenType::Concatenation}};

	int Processor::expressionOperatorPrecedenceGetLevel(Token *token) {
		int index;
		int expressionOperatorPrecedenceLn = sizeof(expressionOperatorPrecedenceTable) / sizeof(ExpressionOperatorPrecedence);
		for (index = 0; index < expressionOperatorPrecedenceLn; ++index) {
			if (expressionOperatorPrecedenceTable[index].subType == token->subType) {
				return expressionOperatorPrecedenceTable[index].level;
			};
		};
		return -1;
	};

	bool Processor::expressionOperatorPrecedenceScan(Token *termBegin, int level, Token *&termEnd, int &nextLevel) {
		Token *eTerm = nullptr;
		Token *eOperator = nullptr;
		Token *index = termBegin;
		bool found = false;
		while (findByTypeNext2(index, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, eTerm)) {
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

	void Processor::expressionOperatorPrecedenceSubstitute(Token *termBegin, Token *termEnd, Token *&newTerm) {
		Token *scan;

		Token *parent = termBegin->parent;
		Token *anchor = termBegin->back;
		TokenList::extractList(parent, termBegin, termEnd);

		newTerm = TokenList::newNode();
		newTerm->type = TokenType::ExpressionPrecedence;
		TokenList::addListToTail(newTerm, termBegin, termEnd);
		TokenList::insertNode(parent, anchor, newTerm);
	};

	void Processor::expressionOperatorPrecedenceLevel(Token *token, int lastLevel, int level) {
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
					if (findByTypeNext2(eOperator, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, eTerm)) {
						if (findByTypeNext(eTerm, TokenType::ExpressionOperator, eOperator)) {
							nextLevel = expressionOperatorPrecedenceGetLevel(eOperator);
							expressionOperatorPrecedenceLevel(eTerm, level, nextLevel);
						};
					};
				};
			};
		};
	};

	void Processor::expressionOperatorPrecedenceSubType(Token *&token) {
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

	void Processor::expressionOperatorPrecedence(Token *&token) {
		expressionOperatorPrecedenceSubType(token);

		Token *eOperator;
		if (findByTypeNext(token->childHead, TokenType::ExpressionOperator, eOperator)) {
			int level = expressionOperatorPrecedenceGetLevel(eOperator);
			expressionOperatorPrecedenceLevel(token->childHead, level, level);
		};
	};

	String Processor::toString(int x) {
		char buffer[24];
		sprintf(buffer, "%d", x);
		return buffer;
	};

	void Processor::removeNoise() {
		Token *scan;
		Token *node;

		for (scan = TokenList::begin(tokenList); scan != nullptr;) {
			node = scan;
			if ((node->type == TokenType::WhiteSpace) || (node->type == TokenType::Comment) || (node->type == TokenType::BOM) || (node->type == TokenType::WhiteSpaceOrComment) || (node->type == TokenType::WS) || (node->type == TokenType::StatementEnd) || (node->type == TokenType::GroupBegin) || (node->type == TokenType::GroupEnd) || (node->type == TokenType::IntervalBegin) || (node->type == TokenType::IntervalSeparator) || (node->type == TokenType::IntervalEnd) || (node->type == TokenType::CountIntervalBegin) || (node->type == TokenType::CountIntervalSeparator) || (node->type == TokenType::CountIntervalEnd)) {
				scan = TokenList::successorNoChild(scan);
				TokenList::extract(node->parent, node);
				TokenList::destructor(node);
				continue;
			};
			scan = TokenList::successor(scan);
		};
	};

	void Processor::transform(Token *token) {
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

	void Processor::orderOperations() {
		Token *scan;
		Token *node;
		Token *term;
		for (scan = TokenList::begin(tokenList); scan != nullptr;) {
			node = scan;

			if ((node->type == TokenType::ZeroOrOne) || (node->type == TokenType::ZeroOrMore) || (node->type == TokenType::OneOrMore) || (node->type == TokenType::CountInterval)) {
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

	void Processor::statementOperations() {
		Token *scan;
		Token *node;
		Token *term;

		node = TokenList::begin(tokenList);
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
							for (index = TokenList::begin(tokenList)->childHead; index != nullptr; index = index->next) {
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

	void Processor::statementExpressionPrecedence() {
		Token *scan;
		Token *node;
		Token *term;

		node = TokenList::begin(tokenList);
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

	String Processor::incFunctionCodeId(const String &symbol) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->codeId++;

		codeInfo.set(symbol, info);

		return toString(info->codeId);
	};

	void Processor::setFunctionHeader(const String &symbol, const String &functionHeader) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->headerFn += functionHeader;

		codeInfo.set(symbol, info);
	};

	void Processor::setFunctionHeaderIntern(const String &symbol, const String &functionHeader) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->headerFnIntern += functionHeader;

		codeInfo.set(symbol, info);
	};

	void Processor::setFunctionCode(const String &symbol, const String &functionCode) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->codeFn += functionCode;

		codeInfo.set(symbol, info);
	};

	void Processor::setFunctionNoCode(const String &symbol) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->noCode = true;

		codeInfo.set(symbol, info);
	};

	bool Processor::getValue(Token *symbol, Token *token) {
		if (token == nullptr) {
			return false;
		};

		if (token->type == TokenType::Number) {
			code += token->value;
			return true;
		};
		if (token->type == TokenType::String) {

			code += "parser.inputIsToken(";
			code += token->value;
			code += ",TokenType::";
			code += symbol->value;
			code += ",node)";

			return true;
		};
		if (token->type == TokenType::Symbol) {
			code += "is";
			code += token->value;
			code += "(parser,node,level)";
			return true;
		};
		if (token->type == TokenType::Any) {
			code += "parser.inputIsTokenAny(";
			code += "TokenType::";
			code += symbol->value;
			code += ",node)";
			return true;
		};
		if (token->type == TokenType::ExpressionTerm) {

			if (token->childHead == nullptr) {
				return false;
			};

			return getValue(symbol, token->childHead);
		};
		if (token->type == TokenType::Group) {

			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "Parentheses";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			String saveCode = code;
			code = "";
			if (!expression(symbol, token->childHead->childHead)) {
				code = saveCode;
				return false;
			};

			functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
			functionCode += "\t\tnode->type = ";
			functionCode += "TokenType::";
			functionCode += symbol->value;
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\tTokenList::transferChild(token,node);\r\n";
			functionCode += "\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\treturn true;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\r\n";
			functionCode += "\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn false;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};
		if (token->type == TokenType::Interval) {
			Token *str1;
			Token *str2;

			if (!findByTypeNext(token->childHead, TokenType::String, str1)) {
				return false;
			};

			if (!findByTypeNext(str1->next, TokenType::String, str2)) {
				return false;
			};

			if (str1->value.length() == 0) {
				return false;
			};
			if (str2->value.length() == 0) {
				return false;
			};
			if (str1->value.length() != str2->value.length()) {
				return false;
			};

			code += "parser.inputIsBetween(";
			code += str1->value;
			code += ",";
			code += str2->value;
			code += ",TokenType::";
			code += symbol->value;
			code += ",node)";

			return true;
		};

		if (token->type == TokenType::Negation) {

			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "Not";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			String saveCode = code;
			code = "";
			if (!getTermValue(symbol, token->next)) {
				code = saveCode;
				return false;
			};

			functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
			functionCode += "\r\n";
			functionCode += "\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\treturn false;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};

		if (token->type == TokenType::ZeroOrOne) {
			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "ZeroOrOne";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			if (token->childHead == nullptr) {
				return false;
			};

			String saveCode = code;
			code = "";
			if (!getValue(symbol, token->childHead)) {
				code = saveCode;
				return false;
			};

			functionCode += "\t\tToken *node  = token;\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\treturn true;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};

		if (token->type == TokenType::ZeroOrMore) {
			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "ZeroOrMore";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			if (token->childHead == nullptr) {
				return false;
			};

			String saveCode = code;
			code = "";
			if (!getValue(symbol, token->childHead)) {
				code = saveCode;
				return false;
			};

			functionCode += "\t\tToken *node  = token;\r\n";
			functionCode += "\r\n";

			functionCode += "\t\twhile(!parser.input.isEof()){\r\n";
			functionCode += "\t\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\t\tcontinue;\r\n";
			functionCode += "\t\t\t};\r\n";
			functionCode += "\t\t\tbreak;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};

		if (token->type == TokenType::OneOrMore) {
			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "OneOrMore";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			if (token->childHead == nullptr) {
				return false;
			};

			String saveCode = code;
			code = "";
			if (!getValue(symbol, token->childHead)) {
				code = saveCode;
				return false;
			};

			functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
			functionCode += "\t\tnode->type = TokenType::";
			functionCode += symbol->value;
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(!(";
			functionCode += code;
			functionCode += ")){\r\n";
			functionCode += "\t\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\treturn false;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\twhile(!parser.input.isEof()){\r\n";
			functionCode += "\t\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\t\tcontinue;\r\n";
			functionCode += "\t\t\t};\r\n";
			functionCode += "\t\t\tbreak;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\tTokenList::transferChild(token,node);\r\n";
			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};

		if (token->type == TokenType::CountInterval) {
			Token *nr1;
			Token *nr2;

			if (!findByTypeNext(token->childHead, TokenType::Number, nr1)) {
				return false;
			};

			if (!findByTypeNext(nr1->next, TokenType::Number, nr2)) {
				String functionId = incFunctionCodeId(symbol->value);
				String functionName;
				String functionHeader;
				String functionCode;

				functionName = "is";
				functionName += symbol->value;
				functionName += "RepeatM";
				functionName += functionId;

				functionHeader = "\tstatic bool ";
				functionHeader += functionName;
				functionHeader += "(Parser &parser, Token *token, int level)";

				functionCode = functionHeader;
				functionCode += "{\r\n";

				functionHeader += ";\r\n";

				String saveCode = code;
				code = "";

				if (token->childHead == nullptr) {
					return false;
				};

				if (!getValue(symbol, token->childHead)) {
					code = saveCode;
					return false;
				};

				functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
				functionCode += "\t\tnode->type = TokenType::";
				functionCode += symbol->value;
				functionCode += ";\r\n";
				functionCode += "\r\n";

				functionCode += "\t\tint m;";
				functionCode += "\t\tfor(m=0;m<";
				functionCode += nr1->value;
				functionCode += ";m++){\r\n";
				functionCode += "\t\t\t\tif(";
				functionCode += code;
				functionCode += "){\r\n";
				functionCode += "\t\t\t\t\tcontinue;\r\n";
				functionCode += "\t\t\t\t};\r\n";
				functionCode += "\t\t\tparser.inputRestore(node);\r\n";
				functionCode += "\t\t\tTokenList::destructor(node);\r\n";
				functionCode += "\t\t\treturn false;\r\n";
				functionCode += "\t\t\t};\r\n";

				functionCode += "\t\twhile(!parser.input.isEof()){\r\n";
				functionCode += "\t\t\tif(";
				functionCode += code;
				functionCode += "){\r\n";
				functionCode += "\t\t\t\tcontinue;\r\n";
				functionCode += "\t\t\t};\r\n";
				functionCode += "\t\t\tbreak;\r\n";
				functionCode += "\t\t};\r\n";

				functionCode += "\t\tTokenList::transferChild(token,node);\r\n";
				functionCode += "\t\tTokenList::destructor(node);\r\n";
				functionCode += "\t\treturn true;\r\n";
				functionCode += "\t};\r\n";
				functionCode += "\r\n";

				code = saveCode;
				code += functionName;
				code += "(parser,node,level)";

				setFunctionHeaderIntern(symbol->value, functionHeader);
				setFunctionCode(symbol->value, functionCode);

				return true;
			};

			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "RepeatMN";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			String saveCode = code;
			code = "";

			if (token->childHead == nullptr) {
				return false;
			};

			if (!getValue(symbol, token->childHead)) {
				code = saveCode;
				return false;
			};

			functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
			functionCode += "\t\tnode->type = TokenType::";
			functionCode += symbol->value;
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tint mn;";
			functionCode += "\t\tfor(mn=0;mn<";
			functionCode += nr2->value;
			functionCode += ";mn++){\r\n";
			functionCode += "\t\t\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\t\t\tcontinue;\r\n";
			functionCode += "\t\t\t\t};\r\n";
			functionCode += "\t\t\t\tif(mn>=";
			functionCode += nr1->value;
			functionCode += "){";
			functionCode += "\t\t\t\tTokenList::transferChild(token,node);\r\n";
			functionCode += "\t\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\t\treturn true;\r\n";
			functionCode += "\t\t\t\t};\r\n";
			functionCode += "\t\t\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\t\treturn false;\r\n";
			functionCode += "\t\t\t};\r\n";

			functionCode += "\t\tTokenList::transferChild(token,node);\r\n";
			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};

		return false;
	};

	bool Processor::getTermValue(Token *symbol, Token *token) {
		if (token->type == TokenType::ExpressionTerm) {
			if (token->childHead == nullptr) {
				Debug::tokenPrintList(token);
				fflush(stdout);
				return false;
			};
			return getValue(symbol, token->childHead);
		};
		if (token->type == TokenType::ExpressionPrecedence) {
			// true && true && false => false - restore all

			String functionId = incFunctionCodeId(symbol->value);
			String functionName;
			String functionHeader;
			String functionCode;

			functionName = "is";
			functionName += symbol->value;
			functionName += "Precedence";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += "{\r\n";

			functionHeader += ";\r\n";

			String saveCode = code;
			code = "";
			bool retV = expression(symbol, token->childHead);

			functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
			functionCode += "\t\tnode->type = ";
			functionCode += "TokenType::";
			functionCode += symbol->value;
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(";
			functionCode += code;
			functionCode += "){\r\n";
			functionCode += "\t\t\tTokenList::transferChild(token,node);\r\n";
			functionCode += "\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\treturn true;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn false;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser,node,level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return retV;
		};
		return false;
	};

	bool Processor::expression(Token *symbol, Token *token) {

		Token *index;
		Token *op;

		if (findByTypeNext2(token, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, index)) {

			if (!getTermValue(symbol, index)) {
				return false;
			};

			index = index->next;
			for (; index != nullptr; index = index->next) {
				if (findByTypeNext(index, TokenType::ExpressionOperator, op)) {
					if (findByTypeNext2(index->next, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, index)) {
						if (op->subType == TokenType::Alternation) {
							code += "||";
							code += "\r\n\t\t\t";
							if (!getTermValue(symbol, index)) {
								return false;
							};
							continue;
						};
						if (op->subType == TokenType::Concatenation) {
							code += "&&"; // restore all on fail ...
							code += "\r\n\t\t\t";
							if (!getTermValue(symbol, index)) {
								return false;
							};
							continue;
						};
						return false;
					};
				};
			};

			return true;
		};

		return false;
	};

	bool Processor::prepareCode() {

		removeNoise();
		transform(tokenList);
		orderOperations();
		statementOperations();
		statementExpressionPrecedence();

		return true;
	};

	bool Processor::saveCode(const String &sourceNamespace,
	                         const String &sourceIncludeGuard,
	                         const String &sourceIncludePath,
	                         const String &sourceHeaderFile,
	                         const String &outputFolder) {

		TPointer<CodeInfo> info;
		String symbol;
		String headerFn;
		String codeFn;
		size_t i;
		String content;

		String sourceHeaderFileContents;
		String outputFolderX;
		String sourceIncludePathX;

		if (!Shell::fileGetContents(sourceHeaderFile, sourceHeaderFileContents)) {
			return false;
		};

		if (outputFolder.length() > 0) {
			if (!Shell::mkdirRecursively(outputFolder)) {
				return false;
			};

			outputFolderX = outputFolder;
			outputFolderX += Shell::pathSeparator;
		};

		if (sourceIncludePath.length() > 0) {
			sourceIncludePathX = sourceIncludePath;
			sourceIncludePathX += "/";
		};

		Token *index;

		index = TokenList::begin(tokenList);
		if (index == nullptr) {
			return false;
		};

		for (index = index->childHead; index != nullptr; index = index->next) {
			if (index->type == TokenType::Statement) {
				Token *symbol;
				Token *expression;
				if (findByTypeNext(index->childHead, TokenType::Symbol, symbol)) {

					String functionName;
					String functionHeader;
					String functionCode;

					functionName = "is";
					functionName += symbol->value;

					functionHeader = "\tbool ";
					functionHeader += functionName;
					functionHeader += "(Parser &parser, Token *token, int level)";

					functionCode = functionHeader;
					functionCode += "{\r\n";

					functionHeader += ";\r\n";

					if (findByTypeNext(symbol->next, TokenType::Expression, expression)) {

						functionCode += "\t\tToken *node;\r\n";
						functionCode += "\t\tif(!parser.codeBegin(node, TokenType::";
						functionCode += symbol->value;
						functionCode += ", level)){\r\n";
						functionCode += "\t\t\treturn false;\r\n";
						functionCode += "\t\t};\r\n";
						functionCode += "\r\n";

						functionCode += "\t\tif(";

						code = "";
						Processor::expression(symbol, expression->childHead);
						functionCode += code;

						functionCode += "){\r\n";

						functionCode += "\t\t\tparser.codeEndTrue(token,node);\r\n";

						functionCode += "\t\t\treturn true;\r\n";
						functionCode += "\t\t};\r\n";

						functionCode += "\r\n";

						functionCode += "\t\tparser.codeEndFalse(node,TokenType::";
						functionCode += symbol->value;
						functionCode += ");\r\n";
					};

					functionCode += "\t\treturn false;\r\n";
					functionCode += "\t};\r\n\r\n";

					setFunctionHeader(symbol->value, functionHeader);
					setFunctionCode(symbol->value, functionCode);
				};
			};
		};

		setFunctionNoCode("ExpressionOperator");
		setFunctionNoCode("ExpressionPrecedence");

		//
		// Code.hpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		content << "#ifndef " << sourceIncludeGuard << "_CODE_HPP\r\n"
		                                               "#define "
		        << sourceIncludeGuard << "_CODE_HPP\r\n"
		                                 "\r\n"
		                                 "#ifndef "
		        << sourceIncludeGuard << "_TOKENTYPE_HPP\r\n"
		                                 "#	include <"
		        << sourceIncludePathX << "TokenType.hpp>\r\n"
		                                 "#endif\r\n"
		                                 "\r\n"
		                                 "namespace "
		        << sourceNamespace << " {\r\n"
		                              "\tusing namespace XYO::GrammarCompiler;\r\n"
		                              "\r\n";

		for (i = 0; i < codeInfo.length(); ++i) {
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};
			content += info->headerFn;
		};

		content += "\r\n"
		           "};\r\n"
		           "\r\n"
		           "#endif\r\n"
		           "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "Code.hpp", content)) {
			return false;
		};

		// [Statement].cpp

		for (i = 0; i < codeInfo.length(); ++i) {
			symbol = (*codeInfo.arrayKey)[i];
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};
			if (info->noCode) {
				continue;
			};

			content = sourceHeaderFileContents;
			content += "\r\n";

			content << "#include <" << sourceIncludePathX << "Code.hpp>\r\n"
			                                                 "\r\n"
			                                                 "namespace "
			        << sourceNamespace << " {\r\n"
			                              "\t\r\n"
			                              "\t\r\n"
			                              "\t\r\n"
			                              "\tusing namespace XYO::GrammarCompiler;\r\n"
			                              "\r\n";

			content += info->headerFnIntern;
			content += "\r\n";

			content += info->codeFn;

			content += "\r\n"
			           "};\r\n"
			           "\r\n";

			String fileName;

			fileName = outputFolderX + "is";
			fileName += symbol;
			fileName += ".cpp";

			if (!System::Shell::filePutContents(fileName, content)) {
				return false;
			};
		};

		//
		// Code.Amalgam.cpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		for (i = 0; i < codeInfo.length(); ++i) {
			symbol = (*codeInfo.arrayKey)[i];
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};
			if (info->noCode) {
				continue;
			};

			content << "#include <" << sourceIncludePathX << "is" << symbol << ".cpp>\r\n";
		};

		content << "#include <" << sourceIncludePathX << "ParserTable.cpp>\r\n";
		content << "#include <" << sourceIncludePathX << "TokenType.cpp>\r\n";

		content += "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "Code.Amalgam.cpp", content)) {
			return false;
		};

		//
		// TokenType.cpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		content << "#include <" << sourceIncludePathX << "TokenType.hpp>\r\n";
		content += "\r\n";
		content << "namespace " << sourceNamespace << " {\r\n";
		content += "\tusing namespace XYO::GrammarCompiler;\r\n";
		content += "\r\n";
		content += "#define TOKENTYPE_STR_B(X) #X\r\n";
		content += "#define TOKENTYPE_STR_A(X) TOKENTYPE_STR_B(X)\r\n";
		content += "#define TOKENTYPE_STR(name) case TokenType::name:return TOKENTYPE_STR_A(name);break\r\n";
		content += "\r\n";
		content += "\tconst char *getTokenTypeString(Token::Type type){\r\n";
		content += "\t\tswitch (type) {\r\n";
		content += "\t\tTOKENTYPE_STR(Unknown);\r\n";

		for (i = 0; i < codeInfo.length(); ++i) {
			symbol = (*codeInfo.arrayKey)[i];
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};
			content += "\t\tTOKENTYPE_STR(";
			content += symbol;
			content += ");\r\n";
		};

		content += "\t\tdefault:\r\n";
		content += "\t\t\tbreak;\r\n";
		content += "\t\t};\r\n";
		content += "\r\n";
		content += "\t\treturn \"Unknown\";\r\n";
		content += "\t};\r\n";
		content += "\r\n";
		content += "};\r\n";
		content += "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "TokenType.cpp", content)) {
			return false;
		};

		//
		// TokenType.hpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		content << "#ifndef " << sourceIncludeGuard << "_TOKENTYPE_HPP\r\n";
		content << "#define " << sourceIncludeGuard << "_TOKENTYPE_HPP\r\n";
		content += "\r\n";
		content += "#ifndef XYO_GRAMMARCOMPILER_HPP\r\n";
		content += "#	include <XYO/GrammarCompiler.hpp>\r\n";
		content += "#endif\r\n";
		content += "\r\n";
		content << "namespace " << sourceNamespace << " {\r\n";
		content += "\tusing namespace XYO::GrammarCompiler;\r\n";
		content += "\r\n";
		content += "\tstruct TokenType {\r\n";
		content += "\t\t\tstatic const Token::Type Unknown = 0;\r\n";

		for (i = 0; i < codeInfo.length(); ++i) {
			symbol = (*codeInfo.arrayKey)[i];
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};
			content += "\t\t\tstatic const Token::Type ";
			content += symbol;
			content += " = ";
			content += toString(i + 1);
			content += ";\r\n";
		};

		content += "\t};\r\n";
		content += "\r\n";
		content += "\tconst char *getTokenTypeString(Token::Type type);\r\n";
		content += "};\r\n";
		content += "\r\n";
		content += "#endif\r\n";
		content += "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "TokenType.hpp", content)) {
			return false;
		};

		//
		// ParserTable.hpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		content << "#ifndef " << sourceIncludeGuard << "_PARSERTABLE_HPP\r\n";
		content << "#define " << sourceIncludeGuard << "_PARSERTABLE_HPP\r\n";
		content += "\r\n";
		content += "#ifndef XYO_GRAMMARCOMPILER_HPP\r\n";
		content += "#	include <XYO/GrammarCompiler.hpp>\r\n";
		content += "#endif\r\n";
		content += "\r\n";
		content << "namespace " << sourceNamespace << " {\r\n";
		content += "\t\r\n";
		content += "\t\r\n";
		content += "\t\r\n";
		content += "\tusing namespace XYO::GrammarCompiler;\r\n";
		content += "\r\n";
		content += "\tParserProcessToken parserTable[];\r\n";
		content += "};\r\n";
		content += "\r\n";
		content += "#endif\r\n";
		content += "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "ParserTable.hpp", content)) {
			return false;
		};

		//
		// ParserTable.cpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		content << "#include <" << sourceIncludePathX << "Code.hpp>\r\n";
		content << "#include <" << sourceIncludePathX << "ParserTable.hpp>\r\n";
		content += "\r\n";
		content << "namespace " << sourceNamespace << " {\r\n";
		content += "\tusing namespace XYO::GrammarCompiler;\r\n";
		content += "\r\n";
		content += "ParserProcessToken parserTable[]={\r\n";

		int codeLn = codeInfo.length();
		for (i = 0; i < codeLn; ++i) {
			symbol = (*codeInfo.arrayKey)[codeLn - i - 1];
			info = (*codeInfo.arrayValue)[codeLn - i - 1];
			if (!info) {
				continue;
			};
			if (info->noCode) {
				continue;
			};

			content += "\tis";
			content += symbol;
			content += ",\r\n";
		};

		content += "// End\r\n";
		content += "\tnullptr\r\n";
		content += "};\r\n";
		content += "\r\n";
		content += "};\r\n";
		content += "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "ParserTable.cpp", content)) {
			return false;
		};

		return true;
	};

	size_t Processor::getLineCount() {
		size_t count = 1;
		Token *scan;
		for (scan = TokenList::begin(tokenList); scan != nullptr; scan = TokenList::successor(scan)) {
			if (scan->type == TokenType::NewLine) {
				++count;
			};
		};
		return count;
	};
};
