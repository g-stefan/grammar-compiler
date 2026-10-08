// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_GENERATOR_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_GENERATOR_HPP

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_GRAMMAR_HPP
#	include <XYO/GrammarCompiler.Application/Grammar.hpp>
#endif

namespace XYO::GrammarCompiler::Application {

	class Generator {
		public:
			GrammarNode *grammar;
			String error;

			// current rule
			GrammarNode *rule;
			int functionId;
			String headerFnIntern;
			String codeFnIntern;

		public:
			Generator();
			~Generator();

			String toString(int x);
			String tokenType();
			String newFunction(const char *kind, String &functionCode);
			void endFunction(const String &functionName, String &functionCode);
			String loopCode(const String &code, const char *indent);

			String expression(GrammarNode *node, bool isTop);
			String ruleCode(GrammarNode *rule_);

			bool saveCode(const String &sourceNamespace,
			              const String &sourceIncludeGuard,
			              const String &sourceIncludePath,
			              const String &sourceHeaderFile,
			              const String &outputFolder);
	};

};

#endif
