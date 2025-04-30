// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#define XYO_GRAMMARCOMPILER_DEPENDENCY_HPP

#ifndef XYO_SYSTEM_HPP
#	include <XYO/System.hpp>
#endif

// -- Export

#ifdef GRAMMAR_COMPILER_INTERNAL
#	ifndef XYO_GRAMMARCOMPILER_INTERNAL
#		define XYO_GRAMMARCOMPILER_INTERNAL
#	endif
#endif

#ifdef GRAMMAR_COMPILER_DLL_INTERNAL
#	ifndef XYO_GRAMMARCOMPILER_DLL_INTERNAL
#		define XYO_GRAMMARCOMPILER_DLL_INTERNAL
#	endif
#endif
       
#ifdef XYO_GRAMMARCOMPILER_DLL_INTERNAL
#	define XYO_GRAMMARCOMPILER_EXPORT XYO_PLATFORM_LIBRARY_EXPORT
#else
#	define XYO_GRAMMARCOMPILER_EXPORT XYO_PLATFORM_LIBRARY_IMPORT
#endif

#if XYO_GRAMMARCOMPILER_LIBRARY
#	undef XYO_GRAMMARCOMPILER_EXPORT
#	define XYO_GRAMMARCOMPILER_EXPORT
#endif

// --

namespace XYO::GrammarCompiler {
	using namespace XYO::ManagedMemory;
	using namespace XYO::DataStructures;
	using namespace XYO::Encoding;
	using namespace XYO::System;
};

#endif
