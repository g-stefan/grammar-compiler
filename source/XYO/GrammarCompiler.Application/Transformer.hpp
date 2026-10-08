// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_TRANSFORMER_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_TRANSFORMER_HPP

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_GRAMMAR_HPP
#	include <XYO/GrammarCompiler.Application/Grammar.hpp>
#endif

// Grammar transformations, see input/Grammar.transform.txt
namespace XYO::GrammarCompiler::Application::Transformer {

	struct Context {
			GrammarNode *rule;
			String *warning;
	};

	// node must be detached, returns the simplified node
	GrammarNode *simplify(GrammarNode *node, Context &context);
	void simplifyRule(GrammarNode *rule, Context &context);
	bool removeLeftRecursion(GrammarNode *rule, Context &context, String &error);

	bool isNullable(GrammarNode *node);
	bool isAlwaysSuccess(GrammarNode *node);
	void computeRuleProperties(GrammarNode *grammar);
	bool checkLeftRecursion(GrammarNode *grammar, String &error);
	void removeUnreachable(GrammarNode *node, Context &context);
	void checkLoops(GrammarNode *node, Context &context);

	bool process(GrammarNode *grammar, String &error, String &warning);

};

#endif
