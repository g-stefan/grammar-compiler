// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler.Application/Code/Code.hpp>
#include <XYO/GrammarCompiler.Application/Code/ParserTable.hpp>

namespace XYO::GrammarCompiler::Application::Code {

	ParserProcessToken parserTable[] = {
	    isStatement,
	    isWhiteSpaceOrComment,
	    // End
	    nullptr};

};
