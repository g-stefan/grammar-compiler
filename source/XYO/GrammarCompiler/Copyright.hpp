// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_COPYRIGHT_HPP
#define XYO_GRAMMARCOMPILER_COPYRIGHT_HPP

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#	include <XYO/GrammarCompiler/Dependency.hpp>
#endif

namespace XYO::GrammarCompiler::Copyright {
	XYO_GRAMMARCOMPILER_EXPORT const char *copyright();
	XYO_GRAMMARCOMPILER_EXPORT const char *publisher();
	XYO_GRAMMARCOMPILER_EXPORT const char *company();
	XYO_GRAMMARCOMPILER_EXPORT const char *contact();
};

#endif
