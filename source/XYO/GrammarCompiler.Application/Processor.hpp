// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_PROCESSOR_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_PROCESSOR_HPP

#ifndef XYO_GRAMMARCOMPILER_PARSER_HPP
#	include <XYO/GrammarCompiler/Parser.hpp>
#endif

namespace XYO::GrammarCompiler::Application {

	class CodeInfo : public Object {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(CodeInfo);

		public:
			int codeId;
			String headerFn;
			String headerFnIntern;
			String codeFn;
			bool noCode;

			inline CodeInfo() {
				codeId = 0;
				noCode = false;
			};

			inline void activeConstructor() {
				codeId = 0;
			};
	};

	class Processor {
		public:
			Token *tokenList;

			String code;

			TAssociativeArray<String, TPointer<CodeInfo>> codeInfo;

		public:
			Processor();
			~Processor();

			String toString(int x);

			String incFunctionCodeId(const String &symbol);
			void setFunctionHeader(const String &symbol, const String &functionHeader);
			void setFunctionHeaderIntern(const String &symbol, const String &functionHeader);
			void setFunctionCode(const String &symbol, const String &functionCode);
			void setFunctionNoCode(const String &symbol);

			static bool findByTypeNext(Token *this_, Token::Type type, Token *&token);
			static bool findByTypeNext2(Token *this_, Token::Type type1, Token::Type type2, Token *&token);

			static int expressionOperatorPrecedenceGetLevel(Token *token);
			static bool expressionOperatorPrecedenceScan(Token *termBegin, int level, Token *&termEnd, int &nextLevel);
			static void expressionOperatorPrecedenceSubstitute(Token *termBegin, Token *termEnd, Token *&newTerm);
			static void expressionOperatorPrecedenceLevel(Token *token, int lastLevel, int level);
			static void expressionOperatorPrecedenceSubType(Token *&token);
			static void expressionOperatorPrecedence(Token *&token);

			bool getValue(Token *symbol, Token *token);
			bool getTermValue(Token *symbol, Token *token);
			bool expression(Token *symbol, Token *token);
			void removeNoise();
			void transform(Token *token);
			void orderOperations();
			void statementOperations();
			void statementExpressionPrecedence();

			size_t getLineCount();

			bool prepareCode();
			bool saveCode(const String &sourceNamespace,
			              const String &sourceIncludeGuard,
			              const String &sourceIncludePath,
			              const String &sourceHeaderFile,
			              const String &outputFolder);
	};

};

#endif
