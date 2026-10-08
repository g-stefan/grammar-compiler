// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_INPUT_HPP
#define XYO_GRAMMARCOMPILER_INPUT_HPP

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#	include <XYO/GrammarCompiler/Dependency.hpp>
#endif

namespace XYO::GrammarCompiler {

	// The whole input is kept in memory, backtracking is done by restoring a position.
	class Input {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Input);

		public:
			String content;
			const char *buffer;
			size_t size;
			size_t position;
			size_t farthestPosition;
			char input;

			XYO_GRAMMARCOMPILER_EXPORT Input();
			XYO_GRAMMARCOMPILER_EXPORT ~Input();

			inline operator char() {
				return input;
			};

			inline char value() {
				return input;
			};

			XYO_GRAMMARCOMPILER_EXPORT void setContent(const String &value);
			XYO_GRAMMARCOMPILER_EXPORT size_t getLineNumber(size_t position_);

			inline bool read() {
				if (position < size) {
					++position;
					if (position > farthestPosition) {
						farthestPosition = position;
					};
				};
				if (position < size) {
					input = buffer[position];
					return true;
				};
				input = 0;
				return false;
			};

			inline bool isEof() {
				return (position >= size);
			};

			inline void setPosition(size_t value) {
				position = value;
				if (position < size) {
					input = buffer[position];
					return;
				};
				input = 0;
			};

			// length must not exceed the remaining input
			inline void skip(size_t length) {
				setPosition(position + length);
				if (position > farthestPosition) {
					farthestPosition = position;
				};
			};
	};

};

#endif
