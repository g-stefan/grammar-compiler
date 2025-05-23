// Grammar Compiler
// Copyright (c) 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT
//

#include <XYO/GrammarCompiler/License.hpp>
#include <XYO/GrammarCompiler.Application/License.hpp>

namespace XYO::GrammarCompiler::Application::License {

	std::string license() {
		return XYO::GrammarCompiler::License::license();
	};

	std::string shortLicense() {
		return XYO::GrammarCompiler::License::shortLicense();
	};

};
