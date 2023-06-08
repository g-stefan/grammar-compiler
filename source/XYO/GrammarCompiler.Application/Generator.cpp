// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Generator.hpp>
#include <XYO/GrammarCompiler.Application/Code/Code.hpp>
#include <XYO/GrammarCompiler.Application/TokenTypeExtra.hpp>
#include <XYO/GrammarCompiler.Application/Debug.hpp>
#include <XYO/System.hpp>

namespace XYO::GrammarCompiler::Application {

	using namespace XYO::GrammarCompiler::Application::Code;

	Generator::Generator() {
		tokenList = nullptr;
	};

	Generator::~Generator(){};

	String Generator::toString(int x) {
		char buffer[24];
		sprintf(buffer, "%d", x);
		return buffer;
	};

	String Generator::getSymbol(const String &symbol) {
		if (symbol[0] == '@') {
			return symbol.index(1);
		};
		return symbol;
	};

	String Generator::incFunctionCodeId(const String &symbol) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->codeId++;

		codeInfo.set(symbol, info);

		return toString(info->codeId);
	};

	void Generator::setFunctionHeader(const String &symbol, const String &functionHeader) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->headerFn += functionHeader;

		codeInfo.set(symbol, info);
	};

	void Generator::setFunctionHeaderIntern(const String &symbol, const String &functionHeader) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->headerFnIntern += functionHeader;

		codeInfo.set(symbol, info);
	};

	void Generator::setFunctionCode(const String &symbol, const String &functionCode) {
		TPointer<CodeInfo> info;
		codeInfo.get(symbol, info);
		if (!info) {
			info.newMemory();
		};

		info->codeFn += functionCode;

		codeInfo.set(symbol, info);
	};

	bool Generator::getValue(Token *symbol, Token *token) {
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
			code += ", TokenType::";
			code += getSymbol(symbol->value);
			code += ", node)";

			return true;
		};
		if (token->type == TokenType::Symbol) {
			code += "is";
			code += getSymbol(token->value);
			code += "(parser, node, level)";
			return true;
		};
		if (token->type == TokenType::Any) {
			code += "parser.inputIsTokenAny(";
			code += "TokenType::";
			code += getSymbol(symbol->value);
			code += ", node)";
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
			functionName += getSymbol(symbol->value);
			functionName += "Parentheses";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += " {\r\n";

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
			functionCode += getSymbol(symbol->value);
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(";
			functionCode += code;
			functionCode += ") {\r\n";
			functionCode += "\t\t\tTokenList::transferChild(token, node);\r\n";
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
			code += "(parser, node, level)";

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
			code += ", TokenType::";
			code += getSymbol(symbol->value);
			code += ", node)";

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
			functionCode += " {\r\n";

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
			functionCode += ") {\r\n";
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
			code += "(parser, node, level)";

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
			functionName += getSymbol(symbol->value);
			functionName += "ZeroOrOne";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += " {\r\n";

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
			functionCode += ") {\r\n";
			functionCode += "\t\t\treturn true;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser, node, level)";

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
			functionName += getSymbol(symbol->value);
			functionName += "ZeroOrMore";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += " {\r\n";

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

			functionCode += "\t\twhile(!parser.input.isEof()) {\r\n";
			functionCode += "\t\t\tif(";
			functionCode += code;
			functionCode += ") {\r\n";
			functionCode += "\t\t\t\tcontinue;\r\n";
			functionCode += "\t\t\t};\r\n";
			functionCode += "\t\t\tbreak;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser, node, level)";

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
			functionName += getSymbol(symbol->value);
			functionName += "OneOrMore";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += " {\r\n";

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
			functionCode += getSymbol(symbol->value);
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(!(";
			functionCode += code;
			functionCode += ")) {\r\n";
			functionCode += "\t\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\treturn false;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\twhile(!parser.input.isEof()) {\r\n";
			functionCode += "\t\t\tif(";
			functionCode += code;
			functionCode += ") {\r\n";
			functionCode += "\t\t\t\tcontinue;\r\n";
			functionCode += "\t\t\t};\r\n";
			functionCode += "\t\t\tbreak;\r\n";
			functionCode += "\t\t};\r\n";
			functionCode += "\t\tTokenList::transferChild(token, node);\r\n";
			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser, node, level)";

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
				functionName += getSymbol(symbol->value);
				functionName += "RepeatM";
				functionName += functionId;

				functionHeader = "\tstatic bool ";
				functionHeader += functionName;
				functionHeader += "(Parser &parser, Token *token, int level)";

				functionCode = functionHeader;
				functionCode += " {\r\n";

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
				functionCode += getSymbol(symbol->value);
				functionCode += ";\r\n";
				functionCode += "\r\n";

				functionCode += "\t\tint m;";
				functionCode += "\t\tfor(m = 0; m < ";
				functionCode += nr1->value;
				functionCode += "; m++){\r\n";
				functionCode += "\t\t\t\tif(";
				functionCode += code;
				functionCode += ") {\r\n";
				functionCode += "\t\t\t\t\tcontinue;\r\n";
				functionCode += "\t\t\t\t};\r\n";
				functionCode += "\t\t\tparser.inputRestore(node);\r\n";
				functionCode += "\t\t\tTokenList::destructor(node);\r\n";
				functionCode += "\t\t\treturn false;\r\n";
				functionCode += "\t\t\t};\r\n";

				functionCode += "\t\twhile(!parser.input.isEof()) {\r\n";
				functionCode += "\t\t\tif(";
				functionCode += code;
				functionCode += ") {\r\n";
				functionCode += "\t\t\t\tcontinue;\r\n";
				functionCode += "\t\t\t};\r\n";
				functionCode += "\t\t\tbreak;\r\n";
				functionCode += "\t\t};\r\n";

				functionCode += "\t\tTokenList::transferChild(token, node);\r\n";
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
			functionName += getSymbol(symbol->value);
			functionName += "RepeatMN";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += " {\r\n";

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
			functionCode += getSymbol(symbol->value);
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tint mn;";
			functionCode += "\t\tfor(mn = 0; mn < ";
			functionCode += nr2->value;
			functionCode += "; mn++) {\r\n";
			functionCode += "\t\t\t\tif(";
			functionCode += code;
			functionCode += ") {\r\n";
			functionCode += "\t\t\t\t\tcontinue;\r\n";
			functionCode += "\t\t\t\t};\r\n";
			functionCode += "\t\t\t\tif(mn >= ";
			functionCode += nr1->value;
			functionCode += ") {";
			functionCode += "\t\t\t\tTokenList::transferChild(token, node);\r\n";
			functionCode += "\t\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\t\treturn true;\r\n";
			functionCode += "\t\t\t\t};\r\n";
			functionCode += "\t\t\t\tparser.inputRestore(node);\r\n";
			functionCode += "\t\t\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\t\t\treturn false;\r\n";
			functionCode += "\t\t\t};\r\n";

			functionCode += "\t\tTokenList::transferChild(token, node);\r\n";
			functionCode += "\t\tTokenList::destructor(node);\r\n";
			functionCode += "\t\treturn true;\r\n";
			functionCode += "\t};\r\n";
			functionCode += "\r\n";

			code = saveCode;
			code += functionName;
			code += "(parser, node, level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return true;
		};

		return false;
	};

	bool Generator::getTermValue(Token *symbol, Token *token) {
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
			functionName += getSymbol(symbol->value);
			functionName += "Precedence";
			functionName += functionId;

			functionHeader = "\tstatic bool ";
			functionHeader += functionName;
			functionHeader += "(Parser &parser, Token *token, int level)";

			functionCode = functionHeader;
			functionCode += " {\r\n";

			functionHeader += ";\r\n";

			String saveCode = code;
			code = "";
			bool retV = expression(symbol, token->childHead);

			functionCode += "\t\tToken *node  = TokenList::newNode();\r\n";
			functionCode += "\t\tnode->type = ";
			functionCode += "TokenType::";
			functionCode += getSymbol(symbol->value);
			functionCode += ";\r\n";
			functionCode += "\r\n";

			functionCode += "\t\tif(";
			functionCode += code;
			functionCode += ") {\r\n";
			functionCode += "\t\t\tTokenList::transferChild(token, node);\r\n";
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
			code += "(parser, node, level)";

			setFunctionHeaderIntern(symbol->value, functionHeader);
			setFunctionCode(symbol->value, functionCode);

			return retV;
		};
		return false;
	};

	bool Generator::expression(Token *symbol, Token *token) {

		Token *index;
		Token *op;

		if (findByTypeNext(token, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, index)) {

			if (!getTermValue(symbol, index)) {
				return false;
			};

			index = index->next;
			for (; index != nullptr; index = index->next) {
				if (findByTypeNext(index, TokenType::ExpressionOperator, op)) {
					if (findByTypeNext(index->next, TokenType::ExpressionTerm, TokenType::ExpressionPrecedence, index)) {
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

	bool Generator::saveCode(const String &sourceNamespace,
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
					functionName += getSymbol(symbol->value);

					functionHeader = "\tbool ";
					functionHeader += functionName;
					functionHeader += "(Parser &parser, Token *token, int level)";

					functionCode = functionHeader;
					functionCode += "{\r\n";

					functionHeader += ";\r\n";

					if (findByTypeNext(symbol->next, TokenType::Expression, expression)) {

						functionCode += "\t\tToken *node;\r\n";
						functionCode += "\t\tif(!parser.codeBegin(node, TokenType::";
						functionCode += getSymbol(symbol->value);
						functionCode += ", level)){\r\n";
						functionCode += "\t\t\treturn false;\r\n";
						functionCode += "\t\t};\r\n";
						functionCode += "\r\n";

						functionCode += "\t\tif(";

						code = "";
						Generator::expression(symbol, expression->childHead);
						functionCode += code;

						functionCode += "){\r\n";

						functionCode += "\t\t\tparser.codeEndTrue(token, node);\r\n";

						functionCode += "\t\t\treturn true;\r\n";
						functionCode += "\t\t};\r\n";

						functionCode += "\r\n";

						functionCode += "\t\tparser.codeEndFalse(node, TokenType::";
						functionCode += getSymbol(symbol->value);
						functionCode += ");\r\n";
					};

					functionCode += "\t\treturn false;\r\n";
					functionCode += "\t};\r\n\r\n";

					setFunctionHeader(symbol->value, functionHeader);
					setFunctionCode(symbol->value, functionCode);
				};
			};
		};

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

			content = sourceHeaderFileContents;
			content += "\r\n";

			content << "#include <" << sourceIncludePathX << "Code.hpp>\r\n";
			content << "\r\n";
			content << "namespace " << sourceNamespace << " {\r\n";
			content << "\tusing namespace XYO::GrammarCompiler;\r\n";
			content << "\r\n";

			if (info->headerFnIntern.length()) {
				content += info->headerFnIntern;
				content += "\r\n";
			};

			content += info->codeFn;

			content += "};\r\n"
			           "\r\n";

			String fileName;

			fileName = outputFolderX + "is";
			fileName += getSymbol(symbol);
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

			content << "#include <" << sourceIncludePathX << "is" << getSymbol(symbol) << ".cpp>\r\n";
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
		content += "\t\t\tTOKENTYPE_STR(Unknown);\r\n";

		for (i = 0; i < codeInfo.length(); ++i) {
			symbol = (*codeInfo.arrayKey)[i];
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};

			content += "\t\t\tTOKENTYPE_STR(";
			content += getSymbol(symbol);
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
		content += "\tnamespace TokenType {\r\n";
		content += "\t\tenum : Token::Type {\r\n";
		content += "\t\t\tUnknown = 0";

		for (i = 0; i < codeInfo.length(); ++i) {
			symbol = (*codeInfo.arrayKey)[i];
			info = (*codeInfo.arrayValue)[i];
			if (!info) {
				continue;
			};
			content += ",\r\n";
			content += "\t\t\t";
			content += getSymbol(symbol);
		};

		content += "\r\n";
		content += "\t\t};\r\n";
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
		content += "\tusing namespace XYO::GrammarCompiler;\r\n";
		content += "\r\n";
		content += "\textern ParserProcessToken parserTable[];\r\n";
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
		content += "\tParserProcessToken parserTable[] = {\r\n";

		int codeLn = codeInfo.length();
		for (i = 0; i < codeLn; ++i) {
			symbol = (*codeInfo.arrayKey)[codeLn - i - 1];
			info = (*codeInfo.arrayValue)[codeLn - i - 1];
			if (!info) {
				continue;
			};

			if (symbol[0] != '@') {
				continue;
			};

			content += "\t\tis";
			content += getSymbol(symbol);
			content += ",\r\n";
		};

		content += "\t\t// End\r\n";
		content += "\t\tnullptr";
		content += "};\r\n";
		content += "\r\n";
		content += "};\r\n";
		content += "\r\n";

		if (!System::Shell::filePutContents(outputFolderX + "ParserTable.cpp", content)) {
			return false;
		};

		return true;
	};

};
