// Grammar Compiler
// Copyright (c) 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_GRAMMARCOMPILER_APPLICATION_GRAMMAR_HPP
#define XYO_GRAMMARCOMPILER_APPLICATION_GRAMMAR_HPP

#ifndef XYO_GRAMMARCOMPILER_PARSER_HPP
#	include <XYO/GrammarCompiler/Parser.hpp>
#endif

namespace XYO::GrammarCompiler::Application {

	// Grammar as a tree, built from the parse tree of a grammar file
	namespace GrammarNodeType {
		enum : uint16_t {
			Unknown = 0,
			Grammar,   // children: Rule
			Rule,      // value: name, one child: expression
			Empty,     // matches without consuming input
			Literal,   // value: C string literal
			Range,     // value, value2: C string literals
			Any,       // any character
			Reference, // value: rule name, rule: the rule
			Sequence,  // A & B & ...
			Choice,    // A | B | ..., first match
			Optional,  // A?
			ZeroOrMore, // A*
			OneOrMore, // A+
			Repeat,    // A{min,max}, max = -1 is unbounded
			Not        // !A
		};
	};

	struct GrammarNode : public TXList5Node<GrammarNode> {
			XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(GrammarNode);

		public:
			uint16_t type;
			String value;
			String value2;
			int min;
			int max;
			GrammarNode *rule;

			// Rule
			bool isParserTable;
			bool isNullable;
			bool isAlwaysSuccess;
			bool isUsed;
			int mark;
			String source;
			String notes;

			GrammarNode();
			~GrammarNode();

			static inline void initMemory() {
				TMemory<String>::initMemory();
			};

			inline void activeConstructor() {
				back = nullptr;
				next = nullptr;
				childHead = nullptr;
				childTail = nullptr;
				parent = nullptr;

				type = 0;
				min = 0;
				max = -1;
				rule = nullptr;
				isParserTable = false;
				isNullable = false;
				isAlwaysSuccess = false;
				isUsed = false;
				mark = 0;
			};

			inline void activeDestructor() {
				value.activeDestructor();
				value2.activeDestructor();
				source.activeDestructor();
				notes.activeDestructor();
			};
	};

};

namespace XYO::ManagedMemory {
	template <>
	struct TMemory<XYO::GrammarCompiler::Application::GrammarNode> : TMemoryPoolActive<XYO::GrammarCompiler::Application::GrammarNode> {
	};
};

namespace XYO::GrammarCompiler::Application {

	typedef TXList5<GrammarNode, TMemoryPoolActive> GrammarNodeList;

	namespace Grammar {

		GrammarNode *newNode(uint16_t type);
		GrammarNode *newNode(uint16_t type, GrammarNode *child);
		GrammarNode *newNode(uint16_t type, GrammarNode *child1, GrammarNode *child2);
		// node must be detached
		void release(GrammarNode *node);
		void moveChildren(GrammarNode *to, GrammarNode *from);
		size_t childCount(GrammarNode *node);
		GrammarNode *copy(GrammarNode *node);
		bool isEqual(GrammarNode *a, GrammarNode *b);

		bool decodeLiteral(const String &literal, std::string &output);
		String encodeLiteral(const std::string &value);

		String toString(GrammarNode *node);
		String toStringRule(GrammarNode *rule, GrammarNode *expression);
		String toStringGrammar(GrammarNode *grammar);

		bool build(Token *tokenList, GrammarNode *&grammar, String &error);
		bool validate(GrammarNode *grammar, String &error, String &warning);

	};

};

#endif
