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

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

};
