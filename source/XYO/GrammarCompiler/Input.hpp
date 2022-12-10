// Grammar Compiler
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_INPUT_HPP
#define XYO_GRAMMARCOMPILER_INPUT_HPP

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#	include <XYO/GrammarCompiler/Dependency.hpp>
#endif

namespace XYO::GrammarCompiler {

	class Input {
			XYO_DISALLOW_COPY_ASSIGN_MOVE(Input);

		public:
			TPointer<IRead> iRead;
			char *inputStack;
			size_t stackSize;
			size_t stackIndex;
			char input;
			bool eof;

			size_t fileIndex;

			XYO_GRAMMARCOMPILER_EXPORT Input(size_t stackSize_ = 16384);
			XYO_GRAMMARCOMPILER_EXPORT ~Input();

			inline operator char() {
				return input;
			};

			inline char value() {
				return input;
			};

			inline Input &operator=(char value) {
				input = value;
				return *this;
			};

			XYO_GRAMMARCOMPILER_EXPORT void setIRead(IRead *value);

			XYO_GRAMMARCOMPILER_EXPORT bool push();
			XYO_GRAMMARCOMPILER_EXPORT bool pop();
			XYO_GRAMMARCOMPILER_EXPORT bool read();
			XYO_GRAMMARCOMPILER_EXPORT bool isEof();
	};

};

#endif
