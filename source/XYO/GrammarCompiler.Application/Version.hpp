// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_VERSION_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_VERSION_HPP

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#	include <XYO/GrammarCompiler/Dependency.hpp>
#endif

namespace XYO::GrammarCompiler::Application::Version {

	const char *version();
	const char *build();
	const char *versionWithBuild();
	const char *datetime();

};

#endif
