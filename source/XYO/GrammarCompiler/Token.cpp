// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler/Token.hpp>

namespace XYO::GrammarCompiler {

	Token::Token() {
		type = TypeUnknown;
		subType = TypeUnknown;

		back = nullptr;
		next = nullptr;
		childHead = nullptr;
		childTail = nullptr;
		parent = nullptr;
	};

	Token::~Token(){};

};
