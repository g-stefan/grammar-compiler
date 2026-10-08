// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/GrammarCompiler/Input.hpp>

namespace XYO::GrammarCompiler {

	Input::Input() {
		buffer = content.value();
		size = 0;
		position = 0;
		farthestPosition = 0;
		input = 0;
	};

	Input::~Input(){};

	void Input::setContent(const String &value) {
		content = value;
		buffer = content.value();
		size = content.length();
		farthestPosition = 0;
		setPosition(0);
	};

	size_t Input::getLineNumber(size_t position_) {
		size_t line = 1;
		size_t k;
		if (position_ > size) {
			position_ = size;
		};
		for (k = 0; k < position_; ++k) {
			if (buffer[k] == '\n') {
				++line;
			};
		};
		return line;
	};

};
