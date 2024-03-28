// Grammar Compiler
// Copyright (c) 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_TOKEN_HPP
#define XYO_GRAMMARCOMPILER_TOKEN_HPP

#ifndef XYO_GRAMMARCOMPILER_DEPENDENCY_HPP
#	include <XYO/GrammarCompiler/Dependency.hpp>
#endif

namespace XYO::GrammarCompiler {

	struct Token : public TXList5Node<Token> {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Token);

		public:
			typedef uint16_t Type;
			static const uint16_t TypeUnknown = 0;

			Type type;
			Type subType;
			String value;

			XYO_GRAMMARCOMPILER_EXPORT Token();
			XYO_GRAMMARCOMPILER_EXPORT ~Token();

			static inline void initMemory() {
				TMemory<String>::initMemory();
			};

			inline void activeConstructor() {
				back = nullptr;
				next = nullptr;
				childHead = nullptr;
				childTail = nullptr;
				parent = nullptr;

				type = TypeUnknown;
				subType = TypeUnknown;
			};

			inline void activeDestructor() {
				value.activeDestructor();
			};
	};

};

namespace XYO::ManagedMemory {
	template <>
	struct TMemory<XYO::GrammarCompiler::Token> : TMemoryPoolActive<XYO::GrammarCompiler::Token> {
	};
};

namespace XYO::GrammarCompiler {

	typedef TXList5<Token, TMemoryPoolActive> TokenList;

};

#endif
