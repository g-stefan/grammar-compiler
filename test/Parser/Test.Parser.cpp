// Grammar Compiler Test
// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

#include <XYO/GrammarCompiler.hpp>
#include <Parser/Loop/ParserTable.hpp>
#include <Parser/Drift/ParserTable.hpp>
#include <Parser/Backtrack/ParserTable.hpp>
#include <Parser/Recursive/ParserTable.hpp>
#include <Parser/Recursive/TokenType.hpp>
#include <Parser/Group/ParserTable.hpp>
#include <Parser/OrAssign/ParserTable.hpp>
#include <Parser/AndAssign/ParserTable.hpp>
#include <Parser/Negation/ParserTable.hpp>
#include <Parser/Interval/ParserTable.hpp>
#include <Parser/Count/ParserTable.hpp>
#include <Parser/LeftRecursion/ParserTable.hpp>
#include <Parser/LongestMatch/ParserTable.hpp>
#include <Parser/LiteralLongestMatch/ParserTable.hpp>
#include <Parser/NotOptimized/ParserTable.hpp>

using namespace XYO::GrammarCompiler;

class Application : public virtual IApplication {
	protected:
		int failed;

		static String repeat(const char *value, size_t count) {
			String retV;
			size_t k;
			for (k = 0; k < count; ++k) {
				retV += value;
			};
			return retV;
		};

		static String tokenText(Token *root) {
			String retV;
			Token *scan;
			for (scan = TokenList::begin(root); scan != nullptr; scan = TokenList::successor(scan)) {
				retV += scan->value;
			};
			return retV;
		};

		void check(const char *name, bool value) {
			printf("%s %s\n", value ? "[ OK ]" : "[FAIL]", name);
			if (!value) {
				++failed;
			};
		};

		void test(const char *name, ParserProcessToken *parserTable, const String &input, bool expected) {
			Parser parser;
			check(name, parser.parseContent(input, parserTable) == expected);
		};

	public:
		inline Application() {
			failed = 0;
		};

		int main(int cmdN, char *cmdS[]) {
			// ("a"?)* must not loop forever
			test("loop: c", TestParser::Loop::parserTable, "c", false);
			test("loop: aab", TestParser::Loop::parserTable, "aab", true);

			// failure memoization must stay valid after backtracking at end of input
			test("drift: ab", TestParser::Drift::parserTable, "ab", true);

			// backtracking over more than 16 KB
			{
				Parser parser;
				String input = repeat("a", 20000);
				check("backtrack: parse 20000 bytes", parser.parseContent(input, TestParser::Backtrack::parserTable));
				check("backtrack: no input lost", tokenText(parser.tokenList) == input);
			};

			// recursive rule keeps nested tokens
			{
				Parser parser;
				check("recursive: parse", parser.parseContent("((x))", TestParser::Recursive::parserTable));
				check("recursive: text", tokenText(parser.tokenList) == "((x))");
				bool hasN = false;
				Token *scan;
				for (scan = TokenList::begin(parser.tokenList); scan != nullptr; scan = TokenList::successor(scan)) {
					if ((scan->type == TestParser::Recursive::TokenType::N) && (scan->value == "x")) {
						hasN = true;
					};
				};
				check("recursive: token N kept", hasN);
			};

			// operator precedence inside groups
			test("group: ab", TestParser::Group::parserTable, "ab", true);
			test("group: ac", TestParser::Group::parserTable, "ac", true);

			// |= and &=
			test("or-assign: a", TestParser::OrAssign::parserTable, "a", true);
			test("or-assign: b", TestParser::OrAssign::parserTable, "b", true);
			test("and-assign: ac", TestParser::AndAssign::parserTable, "ac", true);
			test("and-assign: bc", TestParser::AndAssign::parserTable, "bc", true);
			test("and-assign: a", TestParser::AndAssign::parserTable, "a", false);

			// negation in a parser table symbol
			test("negation: y", TestParser::Negation::parserTable, "y", true);
			test("negation: x", TestParser::Negation::parserTable, "x", false);

			// interval compares unsigned bytes
			test("interval: a", TestParser::Interval::parserTable, "a", false);
			test("interval: \\xC3", TestParser::Interval::parserTable, "\xC3", true);

			// count intervals, numbers are decimal
			test("count: ab", TestParser::Count::parserTable, "ab", false);
			test("count: aab", TestParser::Count::parserTable, "aab", true);
			test("count: aaab", TestParser::Count::parserTable, "aaab", true);
			test("count: aaaab", TestParser::Count::parserTable, "aaaab", false);
			test("count: c{9}d", TestParser::Count::parserTable, repeat("c", 9) + "d", false);
			test("count: c{10}d", TestParser::Count::parserTable, repeat("c", 10) + "d", true);

			// rule 3, left recursion is removed
			test("left recursion: b", TestParser::LeftRecursion::parserTable, "b", true);
			test("left recursion: baa", TestParser::LeftRecursion::parserTable, "baa", true);

			// rule 5, the longest match
			test("longest match: a", TestParser::LongestMatch::parserTable, "a", true);
			test("longest match: ab", TestParser::LongestMatch::parserTable, "ab", true);
			test("literal longest match: abc", TestParser::LiteralLongestMatch::parserTable, "abc", true);
			test("literal longest match: ab", TestParser::LiteralLongestMatch::parserTable, "ab", true);

			// the grammar written without optimizations, factored by the grammar compiler,
			// without factoring each group level is ~10 times slower
			{
				String input = "S = ";
				input += repeat("(", 10);
				input += "\"a\" & \"b\"";
				input += repeat(")", 10);
				input += ";";
				test("not optimized grammar: group depth 10", TestParser::NotOptimized::parserTable, input, true);
			};

			printf("%s\n", failed ? "Failed" : "Passed");
			return failed ? 1 : 0;
		};
};

XYO_APPLICATION_MAIN(Application);
