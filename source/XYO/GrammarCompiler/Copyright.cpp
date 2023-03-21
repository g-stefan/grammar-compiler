// Grammar Compiler
// Copyright (c) 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler/Copyright.hpp>
#include <XYO/GrammarCompiler/Copyright.rh>

namespace XYO::GrammarCompiler::Copyright {

	static const char *copyright_ = XYO_GRAMMARCOMPILER_COPYRIGHT;
	static const char *publisher_ = XYO_GRAMMARCOMPILER_PUBLISHER;
	static const char *company_ = XYO_GRAMMARCOMPILER_COMPANY;
	static const char *contact_ = XYO_GRAMMARCOMPILER_CONTACT;

	std::string copyright() {
		return copyright_;
	};

	std::string publisher() {
		return publisher_;
	};

	std::string company() {
		return company_;
	};

	std::string contact() {
		return contact_;
	};

};
