// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_VERSION_HPP
#define XYO_GRAMMARCOMPILER_VERSION_HPP

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#	include <XYO/GrammarCompiler/Dependency.hpp>
#endif

namespace XYO::GrammarCompiler::Version {

	XYO_GRAMMARCOMPILER_EXPORT const char *version();
	XYO_GRAMMARCOMPILER_EXPORT const char *build();
	XYO_GRAMMARCOMPILER_EXPORT const char *versionWithBuild();
	XYO_GRAMMARCOMPILER_EXPORT const char *datetime();

};

#endif
