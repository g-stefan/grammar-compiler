// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Generator.hpp>

namespace XYO::GrammarCompiler::Application {

	static const char *separator = "\r\n\t\t\t";

	Generator::Generator() {
		grammar = nullptr;
		rule = nullptr;
		functionId = 0;
	};

	Generator::~Generator(){};

	String Generator::toString(int x) {
		char buffer[24];
		snprintf(buffer, sizeof(buffer), "%d", x);
		return buffer;
	};

	String Generator::tokenType() {
		String retV = "TokenType::";
		retV += rule->value;
		return retV;
	};

	// Helper function of the current rule, returns its name
	String Generator::newFunction(const char *kind, String &functionCode) {
		++functionId;
		String functionName;
		functionName << "is" << rule->value << kind << toString(functionId);
		functionCode = "\tstatic bool ";
		functionCode << functionName << "(Parser &parser, Token *token, int level)";
		headerFnIntern << functionCode << ";\r\n";
		functionCode << " {\r\n";
		return functionName;
	};

	void Generator::endFunction(const String &functionName, String &functionCode) {
		functionCode << "\t};\r\n\r\n";
		codeFnIntern << functionCode;
	};

	// Repeat code while it matches and consumes input, requires loopPosition
	String Generator::loopCode(const String &code, const char *indent) {
		String retV;
		retV << indent << "while(!parser.input.isEof()) {\r\n";
		retV << indent << "\tloopPosition = parser.inputPosition();\r\n";
		retV << indent << "\tif(!(" << code << ")) {\r\n";
		retV << indent << "\t\tbreak;\r\n";
		retV << indent << "\t};\r\n";
		retV << indent << "\tif(parser.inputPosition() == loopPosition) {\r\n";
		retV << indent << "\t\tbreak;\r\n";
		retV << indent << "\t};\r\n";
		retV << indent << "};\r\n";
		return retV;
	};

	// Code of a condition that on failure leaves the input and the tokens unchanged,
	// except at the top of the rule, where codeEndFalse restores them
	String Generator::expression(GrammarNode *node, bool isTop) {
		String code;
		String functionCode;
		String functionName;
		GrammarNode *scan;

		switch (node->type) {
		case GrammarNodeType::Empty:
			return "true";

		case GrammarNodeType::Literal:
			code << "parser.inputIsToken(" << node->value << ", " << tokenType() << ", node)";
			return code;

		case GrammarNodeType::Range:
			code << "parser.inputIsBetween(" << node->value << ", " << node->value2 << ", " << tokenType() << ", node)";
			return code;

		case GrammarNodeType::Any:
			code << "parser.inputIsTokenAny(" << tokenType() << ", node)";
			return code;

		case GrammarNodeType::Reference:
			code << "is" << node->value << "(parser, node, level)";
			return code;

		case GrammarNodeType::Choice:
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (scan != node->childHead) {
					code << "||" << separator;
				};
				code << expression(scan, false);
			};
			if (isTop) {
				return code;
			};
			functionName = "(";
			functionName << code << ")";
			return functionName;

		case GrammarNodeType::Optional:
			code << "(" << expression(node->childHead, false) << " || true)";
			return code;

		case GrammarNodeType::Sequence: {
			if (!isTop) {
				functionName = newFunction("Sequence", functionCode);
			};
			for (scan = node->childHead; scan != nullptr; scan = scan->next) {
				if (scan != node->childHead) {
					code << "&&" << separator;
				};
				code << expression(scan, false);
			};
			if (isTop) {
				return code;
			};

			functionCode << "\t\tsize_t position = parser.inputPosition();\r\n";
			functionCode << "\t\tToken *node = TokenList::newNode();\r\n";
			functionCode << "\t\tnode->type = " << tokenType() << ";\r\n";
			functionCode << "\r\n";
			functionCode << "\t\tif(" << code << ") {\r\n";
			functionCode << "\t\t\tParser::tokenTransferChild(token, node);\r\n";
			functionCode << "\t\t\tTokenList::destructor(node);\r\n";
			functionCode << "\t\t\treturn true;\r\n";
			functionCode << "\t\t};\r\n";
			functionCode << "\r\n";
			functionCode << "\t\tparser.inputRestore(position);\r\n";
			functionCode << "\t\tTokenList::destructor(node);\r\n";
			functionCode << "\t\treturn false;\r\n";
			endFunction(functionName, functionCode);
			break;
		};

		case GrammarNodeType::ZeroOrMore:
			functionName = newFunction("ZeroOrMore", functionCode);
			code = expression(node->childHead, false);

			functionCode << "\t\tToken *node = token;\r\n";
			functionCode << "\t\tsize_t loopPosition;\r\n";
			functionCode << "\r\n";
			functionCode << loopCode(code, "\t\t");
			functionCode << "\t\treturn true;\r\n";
			endFunction(functionName, functionCode);
			break;

		case GrammarNodeType::OneOrMore:
			functionName = newFunction("OneOrMore", functionCode);
			code = expression(node->childHead, false);

			functionCode << "\t\tToken *node = token;\r\n";
			functionCode << "\t\tsize_t loopPosition;\r\n";
			functionCode << "\r\n";
			functionCode << "\t\tif(!(" << code << ")) {\r\n";
			functionCode << "\t\t\treturn false;\r\n";
			functionCode << "\t\t};\r\n";
			functionCode << loopCode(code, "\t\t");
			functionCode << "\t\treturn true;\r\n";
			endFunction(functionName, functionCode);
			break;

		case GrammarNodeType::Repeat:
			functionName = newFunction("Repeat", functionCode);
			code = expression(node->childHead, false);

			functionCode << "\t\tsize_t position = parser.inputPosition();\r\n";
			if (node->max != node->min) {
				functionCode << "\t\tsize_t loopPosition;\r\n";
			};
			functionCode << "\t\tToken *node = TokenList::newNode();\r\n";
			functionCode << "\t\tnode->type = " << tokenType() << ";\r\n";
			functionCode << "\t\tint count;\r\n";
			functionCode << "\r\n";
			functionCode << "\t\tfor(count = 0; count < " << toString(node->min) << "; ++count) {\r\n";
			functionCode << "\t\t\tif(" << code << ") {\r\n";
			functionCode << "\t\t\t\tcontinue;\r\n";
			functionCode << "\t\t\t};\r\n";
			functionCode << "\t\t\tparser.inputRestore(position);\r\n";
			functionCode << "\t\t\tTokenList::destructor(node);\r\n";
			functionCode << "\t\t\treturn false;\r\n";
			functionCode << "\t\t};\r\n";
			if (node->max < 0) {
				functionCode << loopCode(code, "\t\t");
			} else if (node->max > node->min) {
				functionCode << "\t\tfor(; count < " << toString(node->max) << "; ++count) {\r\n";
				functionCode << "\t\t\tloopPosition = parser.inputPosition();\r\n";
				functionCode << "\t\t\tif(!(" << code << ")) {\r\n";
				functionCode << "\t\t\t\tbreak;\r\n";
				functionCode << "\t\t\t};\r\n";
				functionCode << "\t\t\tif(parser.inputPosition() == loopPosition) {\r\n";
				functionCode << "\t\t\t\tbreak;\r\n";
				functionCode << "\t\t\t};\r\n";
				functionCode << "\t\t};\r\n";
			};
			functionCode << "\t\tParser::tokenTransferChild(token, node);\r\n";
			functionCode << "\t\tTokenList::destructor(node);\r\n";
			functionCode << "\t\treturn true;\r\n";
			endFunction(functionName, functionCode);
			break;

		case GrammarNodeType::Not:
			functionName = newFunction("Not", functionCode);
			code = expression(node->childHead, false);

			functionCode << "\t\tsize_t position = parser.inputPosition();\r\n";
			functionCode << "\t\tToken *node = TokenList::newNode();\r\n";
			functionCode << "\r\n";
			functionCode << "\t\tif(" << code << ") {\r\n";
			functionCode << "\t\t\tparser.inputRestore(position);\r\n";
			functionCode << "\t\t\tTokenList::destructor(node);\r\n";
			functionCode << "\t\t\treturn false;\r\n";
			functionCode << "\t\t};\r\n";
			functionCode << "\r\n";
			functionCode << "\t\tTokenList::destructor(node);\r\n";
			functionCode << "\t\treturn true;\r\n";
			endFunction(functionName, functionCode);
			break;

		default:
			error = "Unknown grammar node";
			return "false";
		};

		functionName << "(parser, node, level)";
		return functionName;
	};

	static String comment(const String &text) {
		String retV = "\t// ";
		const char *scan;
		for (scan = text.value(); *scan != 0; ++scan) {
			if ((*scan == '\r') || (*scan == '\n')) {
				retV += ' ';
				continue;
			};
			retV += *scan;
		};
		retV += "\r\n";
		return retV;
	};

	// Helper functions and the function of the rule
	String Generator::ruleCode(GrammarNode *rule_) {
		String functionCode;
		String code;

		rule = rule_;
		functionId = 0;
		headerFnIntern = "";
		codeFnIntern = "";

		code = expression(rule->childHead, true);

		String source;
		if (rule->isParserTable) {
			source += "@";
		};
		source << rule->value << " = " << rule->source << ";";
		functionCode << comment(source);

		String optimized = Grammar::toStringRule(rule, rule->childHead);
		if (optimized != source) {
			String text = "Optimized: ";
			text << optimized;
			functionCode << comment(text);
		};

		const char *scan = rule->notes.value();
		while (*scan != 0) {
			String text = "Note: ";
			for (; (*scan != 0) && (*scan != '\r') && (*scan != '\n'); ++scan) {
				text += *scan;
			};
			for (; (*scan == '\r') || (*scan == '\n'); ++scan) {
			};
			functionCode << comment(text);
		};

		functionCode << "\tbool is" << rule->value << "(Parser &parser, Token *token, int level) {\r\n";
		functionCode << "\t\tToken *node;\r\n";
		functionCode << "\t\tsize_t position;\r\n";
		functionCode << "\t\tif(!parser.codeBegin(node, " << tokenType() << ", level, position)) {\r\n";
		functionCode << "\t\t\treturn false;\r\n";
		functionCode << "\t\t};\r\n";
		functionCode << "\r\n";
		functionCode << "\t\tif(" << code << ") {\r\n";
		functionCode << "\t\t\tparser.codeEndTrue(token, node);\r\n";
		functionCode << "\t\t\treturn true;\r\n";
		functionCode << "\t\t};\r\n";
		functionCode << "\r\n";
		functionCode << "\t\tparser.codeEndFalse(node, " << tokenType() << ", position);\r\n";
		functionCode << "\t\treturn false;\r\n";
		functionCode << "\t};\r\n";
		functionCode << "\r\n";

		String retV;
		if (headerFnIntern.length() > 0) {
			retV << headerFnIntern << "\r\n";
		};
		retV << codeFnIntern << functionCode;
		return retV;
	};

	bool Generator::saveCode(const String &sourceNamespace,
	                         const String &sourceIncludeGuard,
	                         const String &sourceIncludePath,
	                         const String &sourceHeaderFile,
	                         const String &outputFolder) {

		String content;
		String fileName;
		String sourceHeaderFileContents;
		String outputFolderX;
		String sourceIncludePathX;
		GrammarNode *scan;

		if (!Shell::fileGetContents(sourceHeaderFile, sourceHeaderFileContents)) {
			error = "Unable to read source header file ";
			error << sourceHeaderFile;
			return false;
		};

		if (outputFolder.length() > 0) {
			if (!Shell::mkdirRecursively(outputFolder)) {
				error = "Unable to create output folder ";
				error << outputFolder;
				return false;
			};

			outputFolderX = outputFolder;
			outputFolderX += Shell::pathSeparator;
		};

		if (sourceIncludePath.length() > 0) {
			sourceIncludePathX = sourceIncludePath;
			sourceIncludePathX += "/";
		};

		//
		// Code.hpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		content << "#ifndef " << sourceIncludeGuard << "_CODE_HPP\r\n";
		content << "#define " << sourceIncludeGuard << "_CODE_HPP\r\n";
		content += "\r\n";
		content << "#ifndef " << sourceIncludeGuard << "_TOKENTYPE_HPP\r\n";
		content << "#	include <" << sourceIncludePathX << "TokenType.hpp>\r\n";
		content += "#endif\r\n";
		content += "\r\n";
		content << "namespace " << sourceNamespace << " {\r\n";
		content += "\tusing namespace XYO::GrammarCompiler;\r\n";
		content += "\r\n";

		for (scan = grammar->childHead; scan != nullptr; scan = scan->next) {
			content << "\tbool is" << scan->value << "(Parser &parser, Token *token, int level);\r\n";
		};

		content += "\r\n";
		content += "};\r\n";
		content += "\r\n";
		content += "#endif\r\n";
		content += "\r\n";

		fileName = outputFolderX + "Code.hpp";
		if (!Shell::filePutContents(fileName, content)) {
			error = "Unable to write file ";
			error << fileName;
			return false;
		};

		//
		// is[Rule].cpp
		//

		for (scan = grammar->childHead; scan != nullptr; scan = scan->next) {
			content = sourceHeaderFileContents;
			content += "\r\n";

			content << "#include <" << sourceIncludePathX << "Code.hpp>\r\n";
			content += "\r\n";
			content << "namespace " << sourceNamespace << " {\r\n";
			content += "\tusing namespace XYO::GrammarCompiler;\r\n";
			content += "\r\n";
			content += ruleCode(scan);
			content += "};\r\n";
			content += "\r\n";

			if (error.length() > 0) {
				return false;
			};

			fileName = outputFolderX + "is";
			fileName << scan->value << ".cpp";
			if (!Shell::filePutContents(fileName, content)) {
				error = "Unable to write file ";
				error << fileName;
				return false;
			};
		};

		//
		// Code.Amalgam.cpp
		//

		content = sourceHeaderFileContents;
		content += "\r\n";

		for (scan = grammar->childHead; scan != nullptr; scan = scan->next) {
			content << "#include <" << sourceIncludePathX << "is" << scan->value << ".cpp>\r\n";
		};

		content << "#include <" << sourceIncludePathX << "ParserTable.cpp>\r\n";
		content << "#include <" << sourceIncludePathX << "TokenType.cpp>\r\n";
		content += "\r\n";

		fileName = outputFolderX + "Code.Amalgam.cpp";
		if (!Shell::filePutContents(fileName, content)) {
			error = "Unable to write file ";
			error << fileName;
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

		for (scan = grammar->childHead; scan != nullptr; scan = scan->next) {
			content << "\t\t\tTOKENTYPE_STR(" << scan->value << ");\r\n";
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

		fileName = outputFolderX + "TokenType.cpp";
		if (!Shell::filePutContents(fileName, content)) {
			error = "Unable to write file ";
			error << fileName;
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

		for (scan = grammar->childHead; scan != nullptr; scan = scan->next) {
			content << ",\r\n\t\t\t" << scan->value;
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

		fileName = outputFolderX + "TokenType.hpp";
		if (!Shell::filePutContents(fileName, content)) {
			error = "Unable to write file ";
			error << fileName;
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

		fileName = outputFolderX + "ParserTable.hpp";
		if (!Shell::filePutContents(fileName, content)) {
			error = "Unable to write file ";
			error << fileName;
			return false;
		};

		//
		// ParserTable.cpp, @ symbols, the last defined is tried first
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

		for (scan = grammar->childTail; scan != nullptr; scan = scan->back) {
			if (!scan->isParserTable) {
				continue;
			};
			content << "\t\tis" << scan->value << ",\r\n";
		};

		content += "\t\t// End\r\n";
		content += "\t\tnullptr";
		content += "};\r\n";
		content += "\r\n";
		content += "};\r\n";
		content += "\r\n";

		fileName = outputFolderX + "ParserTable.cpp";
		if (!Shell::filePutContents(fileName, content)) {
			error = "Unable to write file ";
			error << fileName;
			return false;
		};

		return true;
	};

};
