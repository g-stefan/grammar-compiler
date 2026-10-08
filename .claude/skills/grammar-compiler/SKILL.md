---
name: grammar-compiler
description: >-
  How to use grammar-compiler, the XYO parser generator that compiles a
  PEG-like .grammar file into C++ parser code, and its runtime library
  XYO::GrammarCompiler (<XYO/GrammarCompiler.hpp>, on xyo-system): the
  grammar language (Name = e; |= &=; @Name start rules; "literal", ., ["a","z"]
  intervals, !e, ? * + {m} {m,n}, & sequence, | ordered choice, comments);
  PEG semantics (ordered choice, greedy loops, failure memoization,
  parser-stack-overflow); the optimizations, notes, warnings and errors
  (prefix factoring, left recursion, longest match); the command line
  (--grammar, --source-header, --source-namespace, --source-include-guard,
  --source-include-path, --output-folder, --output-grammar, @file) and the
  generated files (Code.hpp, is<Rule>.cpp, TokenType, getTokenTypeString,
  ParserTable); the runtime Parser (parseContent, parseFile, getErrorLine,
  tokenList, initMemory), Input, Token / TokenList token tree,
  ParserProcessToken. Use when writing or reviewing a .grammar file, running
  grammar-compiler, walking a generated token tree, code that includes
  <XYO/GrammarCompiler.hpp> or a generated ParserTable.hpp / TokenType.hpp,
  depends on "grammar-compiler" in fabricare.json, or when working inside
  the grammar-compiler repository.
---

# grammar-compiler

Parser generator of the XYO C++ stack. Purpose: write a language as a
grammar, get **plain C++ code** (one function per rule) that parses it into
a **token tree**, built like any other source of an XYO project. The
runtime library `XYO::GrammarCompiler` sits on `xyo-system` (see the
`xyo-system`, `xyo-managed-memory`, `xyo-data-structures` skills; their
rules apply — `String`, `IApplication`, pools). The compiler is self-hosted:
its grammar is `input/Compiler.grammar`, its parser is the generated
`source/XYO/GrammarCompiler.Application/Code/`.

Full documentation: `docs/` in the grammar-compiler repository
(`X:\Storage\XYO\Gitea\CPP\grammar-compiler\docs` on this machine):
README, getting-started, **grammar**, command-line, **transformations**,
**runtime**, reference. Read the matching page when you need more than this
summary. Ground truth: `input/Compiler.grammar`,
`input/Grammar.transform.txt`, `source/XYO/GrammarCompiler/Parser.cpp`,
`source/XYO/GrammarCompiler.Application/Generator.cpp`, tests in
`test/Parser`, `test/Transform`, `test/Error`.

## Workflow

1. Write `input/My.grammar` with at least one `@` start rule.
2. Check it: `grammar-compiler --grammar=input/My.grammar --output-grammar=out.grammar`
   — read every `Warning:` and `Note:`.
3. Generate (from `source/`, so output folder = include path):
   ```
   grammar-compiler --grammar=../input/My.grammar --source-header=../input/My.Source.Header.hpp
       --source-namespace=My::Code --source-include-guard=MY_CODE
       --source-include-path=My/Code --output-folder=My/Code
   ```
   (usually as an `@../input/My.arguments` file, one option per line, run by
   `fabricare/generate.js` → `fabricare generate`, or by
   `fabricare/make.prepare.js` on every build).
4. `fabricare.json`: add `"My/Code"` to `sourcePath`, `"grammar-compiler"`
   to `dependency`. fabricare skips `Code.Amalgam.cpp`; without fabricare
   compile either it or the `is*.cpp` files, not both.
5. Parse and walk `parser.tokenList`.

## Grammar language — cheat sheet

| Syntax | Meaning |
|--------|---------|
| `Name = e;` | define (once) |
| `Name \|= e;` / `Name &= e;` | `(old) \| (e)` / `(old) & (e)`; only after the `=` |
| `@Name = e;` | start rule, in `parserTable` (last defined first) |
| `"txt"` / `""` | literal bytes (C escapes, copied verbatim to C++) / always matches |
| `.` | any one byte |
| `["a","z"]` | per-byte interval, same-length non-empty bounds, unsigned |
| `(e)` `!e` | group / not-followed-by (consumes nothing, tokens dropped) |
| `e?` `e*` `e+` | optional / zero or more / one or more |
| `e{m}` `e{m,n}` | **at least** m / m..n; exactly m is `e{m,m}`; decimal, ≤ 1000000 |
| `a & b`, `a \| b` | sequence, ordered choice; `&` binds tighter than `\|` |
| `// ...`, `# ...`, `/* */` | comments; a line comment needs a trailing newline |

Precedence: `!` (on the next term) > one postfix per term (`!"x"*` is
`(!"x")*`; `A*?` is a syntax error) > `&` > `|`.

## Hard rules

1. **PEG, not regex / CFG.** Ordered choice: `(X | Y) & "c"` never retries
   `Y` after `X` matched — put longer / more specific alternatives first.
   Loops are greedy and never give back: `.* & "x"` and `"a"* & "a"` never
   match; read to a terminator with `(!T & .)* & T`.
2. **Start rules drive the parse.** `Parser::parse` repeatedly tries the
   table entries (last-defined `@` first); the first that **consumes input**
   wins; stops when none does; success = whole input consumed. Empty input
   = `false`. Whitespace / comments between top-level items must be `@`
   rules themselves (like `@WhiteSpaceOrComment` in `Compiler.grammar`).
3. **Rule names become C++ identifiers** `isName`, `TokenType::Name`:
   `Unknown` is reserved (`TokenType::Unknown = 0`) and C++ keywords are
   rejected — both are compile errors of the grammar.
4. **Literals are C string literals copied verbatim**: must be valid C++;
   `\x` eats all following hex digits (`"\x41BC"` is invalid — use octal
   `"\101BC"`; `"a\x00b"` is byte 11, not NUL); **no NUL bytes** (rejected,
   the runtime uses `strlen`; match NUL with `!["\x01","\xFF"] & .`).
5. **Left recursion**: direct (`E = E & A | B`) is rewritten to
   `E = B & A*` with a note; indirect, hidden behind a nullable prefix
   (`A = WS & A ...` with `WS = " "*`), or with no base alternative is an
   error.
6. **Read the diagnostics.** `Note: ... longest match` means rule 5 changed
   what is matched (`"a" | "ab"` → `"a" & "b"?`). `can never match`
   warnings remove alternatives — usually a wrong order. `repeats an
   expression that can match empty input` — the loop stops on no progress.
   `symbol X is not used` — dead rule.
7. **Always pass `--source-include-guard`** (else guards are `_CODE_HPP`
   ...) and a distinct namespace per grammar. Paths in options are relative
   to the current folder, also in `@files`. Generated files are overwritten
   but stale `isOld.cpp` files are not deleted — remove them when renaming
   rules. Never hand-edit generated code; change the grammar.
8. **Parser usage**: one `Parser` per input (`parseContent` does not clear
   `tokenList`, it appends); call `Parser::initMemory()` from the app's
   static `initMemory`; wrap parsing in `try` / `catch` —
   `std::runtime_error("parser-stack-overflow")` after 1024 nested rule
   calls, nothing catches it. On `false`, report `parser.getErrorLine()`
   (line of `input.farthestPosition`).
9. **Token tree** (`parser.tokenList` = root, type 0): every matching rule
   adds one token of its type to its caller; text matched directly by a
   rule becomes children of the **same type** (merged when consecutive);
   if that leaves one text child, it becomes the rule token's `value` (leaf
   rules like `Number [10]`); groups / loops / `?` add no tokens; `!` adds
   none; a rule matching nothing still adds an empty token (`WS []`).
   Concatenating all values in pre-order gives back the input. Want it in
   the tree → give it its own rule; skip noise types while walking.
10. **Ownership / threads**: tokens live in per-thread pools and die with
    the `Parser`; copy strings out to keep or pass them to another thread.
    Compare `type` with `TokenType::Name`, never with stored numbers (enum
    order = rule definition order). `subType` is free for the application.

## Recipes

Parse a file and walk it:

```cpp
#include <My/Code/ParserTable.hpp>
#include <My/Code/TokenType.hpp>

using namespace XYO::GrammarCompiler;
using namespace My::Code;

static void walk(Token *token) {
	for (Token *scan = token->childHead; scan != nullptr; scan = scan->next) {
		if (scan->type == TokenType::WS) {
			continue;
		};
		printf("%s [%s]\n", getTokenTypeString(scan->type), scan->value.value());
		walk(scan);
	};
};

bool load(const char *fileName) {
	Parser parser;
	try {
		if (!parser.parseFile(fileName, parserTable)) {
			printf("%s: syntax error, line %zu\n", fileName, parser.getErrorLine());
			return false;
		};
	} catch (const std::exception &e) {
		printf("%s: %s\n", fileName, e.what());
		return false;
	};
	walk(parser.tokenList);
	return true;
};
```

Start from a rule that is not `@` (custom table):

```cpp
ParserProcessToken table[] = {isWS, isExpression, nullptr};
parser.parseContent(text, table);
```

Common grammar shapes:

```
NewLine = "\r\n" | "\n";
@Space = (" " | "\t" | NewLine)+;
@LineComment = "//" & (!NewLine & .)* & NewLine?;
WS = (" " | "\t" | "\r" | "\n")*;
IdStart = "_" | ["a","z"] | ["A","Z"];
IdChar = IdStart | ["0","9"];
Keyword = ("if" | "else" | "while") & !IdChar;
Name = !Keyword & IdStart & IdChar*;
Number = ["0","9"]+;
String = "\"" & (!"\"" & (("\\" & .) | .))* & "\"";
Sum = Product & (WS & ("+" | "-") & WS & Product)*;
Product = Factor & (WS & ("*" | "/") & WS & Factor)*;
Factor = Number | Name | "(" & WS & Sum & WS & ")";
Arguments = Sum & (WS & "," & WS & Sum)*;
@Statement = Name & WS & "=" & WS & Sum & WS & ";";
```

## Build and test (inside this repository)

fabricare projects: `grammar-compiler` library (`dll-or-lib`, runtime,
`source/XYO/GrammarCompiler`), `grammar-compiler.application.static`
(compiler without `main`), `grammar-compiler` exe. Dependency:
`xyo-system`. Commands: `fabricare make`, `fabricare test`,
`fabricare install`, `fabricare generate` (regenerates
`source/XYO/GrammarCompiler.Application/Code/` from
`input/Compiler.grammar` with the built compiler in `output/bin`). See the
`fabricare` skill.

`fabricare test` fails if the committed generated parser is stale (run
`fabricare generate`), if the self-rebuilt compiler generates different
code, if a `test/Transform/X.grammar` does not optimize to `X.expected` or
is not stable when optimized again, if `Compiler.not-optimized.grammar`
does not optimize to `Compiler.grammar`'s result, if a `test/Error`
grammar is accepted, or if `test/Parser/Test.Parser.cpp` checks fail.
On Windows run it inside the MSVC environment (`vcvars64.bat`); the test
runs `grammar-compiler.test` from `output/test`, which fails with "not
recognized" if `NoDefaultCurrentDirectoryInExePath` is set.

When changing the project: grammar language changes go in
`input/Compiler.grammar` (and `Compiler.not-optimized.grammar`), then
`fabricare make && fabricare generate && fabricare make`; transformations
in `Transformer.cpp` with a `test/Transform` pair and the list in
`fabricare/test.js`; new errors with a `test/Error` grammar; runtime
behavior with a `test/Parser` grammar + check in `Test.Parser.cpp` +
`Test.Parser.fabricare.json` + `parserTests` in `fabricare/test.js`;
update `input/Grammar.transform.txt` and `docs/`.
