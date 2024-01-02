// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_GENERATOR_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_GENERATOR_HPP

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

			inline CodeInfo() {
				codeId = 0;
			};

			inline void activeConstructor() {
				codeId = 0;
			};
	};

	class Generator {
		public:
			Token *tokenList;
			String code;
			TAssociativeArray<String, TPointer<CodeInfo>> codeInfo;

		public:
			Generator();
			~Generator();

			String toString(int x);

			String incFunctionCodeId(const String &symbol);
			void setFunctionHeader(const String &symbol, const String &functionHeader);
			void setFunctionHeaderIntern(const String &symbol, const String &functionHeader);
			void setFunctionCode(const String &symbol, const String &functionCode);

			bool getValue(Token *symbol, Token *token);
			bool getTermValue(Token *symbol, Token *token);
			bool expression(Token *symbol, Token *token);

			String getSymbol(const String &symbol);

			bool prepareCode();
			bool saveCode(const String &sourceNamespace,
			              const String &sourceIncludeGuard,
			              const String &sourceIncludePath,
			              const String &sourceHeaderFile,
			              const String &outputFolder);
	};

};

#endif
