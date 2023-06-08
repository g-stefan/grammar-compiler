// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_TRANSFORMER_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_TRANSFORMER_HPP

#ifndef XYO_GRAMMARCOMPILER_PARSER_HPP
#	include <XYO/GrammarCompiler/Parser.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Transformer {

	int expressionOperatorPrecedenceGetLevel(Token *token);
	bool expressionOperatorPrecedenceScan(Token *termBegin, int level, Token *&termEnd, int &nextLevel);
	void expressionOperatorPrecedenceSubstitute(Token *termBegin, Token *termEnd, Token *&newTerm);
	void expressionOperatorPrecedenceLevel(Token *token, int lastLevel, int level);
	void expressionOperatorPrecedenceSubType(Token *&token);
	void expressionOperatorPrecedence(Token *&token);

	void removeNoise(Token *&token);
	void transform(Token *&token);
	void orderOperations(Token *&token);
	void statementOperations(Token *&token);
	void statementExpressionPrecedence(Token *&token);

	void process(Token *&token);

	size_t getLineCount(Token *&token);
};

#endif
