# Command line

```
grammar-compiler [options] [@options-file ...]
```

## Options

| Option | Meaning |
|--------|---------|
| `--grammar=file` | the grammar to compile (required) |
| `--source-header=file` | text copied at the beginning of every generated file, usually a license comment (required to generate code) |
| `--source-namespace=name` | C++ namespace of the generated code, may be nested: `My::Parser::Code` (required to generate code) |
| `--source-include-guard=name` | prefix of the include guards: `name_CODE_HPP`, `name_TOKENTYPE_HPP`, `name_PARSERTABLE_HPP` |
| `--source-include-path=path` | folder used in the `#include <path/...>` lines between the generated files |
| `--output-folder=path` | where to write the generated files; created if missing; default: the current folder |
| `--output-grammar=file` | write the optimized grammar to `file` |
| `--help`, `--usage` | print the options and exit |
| `--license` | print the license and exit |
| `--version` | print the version and exit |
| `@file` | read more options from `file` |

What runs depends on the options given:

| Given | Result |
|-------|--------|
| `--grammar`, `--source-header`, `--source-namespace` | C++ code |
| `--grammar`, `--output-grammar` | only the optimized grammar, no code |
| `--grammar`, `--output-grammar`, `--source-header`, `--source-namespace` | both |
| `--grammar` only | error: code generation needs `--source-header` and `--source-namespace` |

Details:

- Paths are relative to the **current folder**, also inside an options
  file.
- An option given twice: the last one wins. Unknown options and arguments
  that do not start with `--` or `@` are ignored.
- Always give `--source-include-guard`: without it the guards are
  `_CODE_HPP`, `_TOKENTYPE_HPP`, ... — reserved names in C++ that also
  collide when a program has two generated parsers.
- `--source-include-path` must be the path of the output folder as seen from
  an include directory of the project that compiles the code. The simplest
  way is to run the compiler from that include directory (`source/` in a
  fabricare project) and give the same path to `--output-folder` and
  `--source-include-path`.

### Options files

`@file` reads `file` and splits it like a command line: options are
separated by spaces or new lines, and double quotes group a value that
contains spaces. Options files can be mixed with options on the command
line; an `@` inside an options file is not expanded.

```
--grammar=../input/Compiler.grammar
--source-header=../input/Code.Source.Header.hpp
--source-namespace="XYO::GrammarCompiler::Application::Code"
--source-include-guard="XYO_GRAMMARCOMPILER_APPLICATION_CODE"
--source-include-path="XYO/GrammarCompiler.Application/Code"
--output-folder="XYO/GrammarCompiler.Application/Code"
```

This is [`input/Code.Generate.arguments`](../input/Code.Generate.arguments),
run from `source/` by `fabricare generate`.

## Generated files

For a grammar with rules `R1 ... Rn` and the options
`--source-namespace=N --source-include-guard=G --source-include-path=P`,
the output folder receives:

| File | Content |
|------|---------|
| `TokenType.hpp` | `namespace N::TokenType { enum : Token::Type { Unknown = 0, R1, ..., Rn }; }` in the order the rules are defined, and `const char *N::getTokenTypeString(Token::Type type);` |
| `TokenType.cpp` | `getTokenTypeString`: the rule name of a token type, `"Unknown"` for anything else |
| `Code.hpp` | `bool N::isR(Parser &parser, Token *token, int level);` for every rule |
| `isR.cpp` | one per rule: the function of the rule and its `static` helper functions |
| `ParserTable.hpp` | `extern ParserProcessToken N::parserTable[];` |
| `ParserTable.cpp` | the `@` rules, the last defined first, ending with `nullptr` |
| `Code.Amalgam.cpp` | `#include`s every `.cpp` above, to compile the parser as one file |

Every file begins with the text of `--source-header`. The headers include
each other as `<P/...>` and pull in `<XYO/GrammarCompiler.hpp>`. Your code
normally includes `ParserTable.hpp` (to parse) and `TokenType.hpp` (to read
the tokens).

Each generated file is overwritten on every run; other files in the output
folder are left alone. When a rule is renamed or removed, delete its old
`isR.cpp` by hand (or empty the folder before generating), otherwise it is
still compiled.

### A generated rule

For the rule `Factor = Number | Name | "(" & WS & Sum & WS & ")";`,
`isFactor.cpp` contains:

```cpp
	static bool isFactorSequence1(Parser &parser, Token *token, int level);

	static bool isFactorSequence1(Parser &parser, Token *token, int level) {
		size_t position = parser.inputPosition();
		Token *node = TokenList::newNode();
		node->type = TokenType::Factor;

		if(parser.inputIsToken("(", TokenType::Factor, node)&&
			isWS(parser, node, level)&&
			isSum(parser, node, level)&&
			isWS(parser, node, level)&&
			parser.inputIsToken(")", TokenType::Factor, node)) {
			Parser::tokenTransferChild(token, node);
			TokenList::destructor(node);
			return true;
		};

		parser.inputRestore(position);
		TokenList::destructor(node);
		return false;
	};

	// Factor = Number | Name | "(" & WS & Sum & WS & ")";
	bool isFactor(Parser &parser, Token *token, int level) {
		Token *node;
		size_t position;
		if(!parser.codeBegin(node, TokenType::Factor, level, position)) {
			return false;
		};

		if(isNumber(parser, node, level)||
			isName(parser, node, level)||
			isFactorSequence1(parser, node, level)) {
			parser.codeEndTrue(token, node);
			return true;
		};

		parser.codeEndFalse(node, TokenType::Factor, position);
		return false;
	};
```

- The comment above the rule function is the rule **as written**. If the
  compiler changed it, an `// Optimized: ...` line follows, and one
  `// Note: ...` line for each transformation that changes what is matched
  (see [Optimizations](transformations.md)).
- Nested parts of the expression become `static` helpers named
  `is<Rule><Kind><N>`, where Kind is `Sequence`, `ZeroOrMore`, `OneOrMore`,
  `Repeat` or `Not`.
- `parser.codeBegin` checks the failure memo and the nesting budget and
  creates the rule's token; `codeEndTrue` adds it to the caller's token;
  `codeEndFalse` restores the input, drops the token and remembers the
  failure. See [Runtime](runtime.md).

## Messages and exit code

The exit code is `0` on success and `1` on any error. Everything is printed
to standard output.

| Message | Cause |
|---------|-------|
| `Error: No grammar file specified!` | missing `--grammar` |
| `Error: No source header file specified!` | code generation without `--source-header` |
| `Error: No source namespace specified!` | code generation without `--source-namespace` (or `--output-grammar`) |
| `Error: Grammar file not found - file` | `--grammar` names a missing file |
| `Error: file not found - file` | `@file` names a missing file |
| `Error: Syntax error line: N!` | the grammar file does not follow the [grammar language](grammar.md); `N` is the line of the farthest position the parser reached |
| `Error: parser-stack-overflow line: N!` | the grammar file nests groups too deeply |
| `Error: Symbol ... ` and others | the grammar is not valid, see [diagnostics](transformations.md#errors) |
| `Error: Unable to read source header file ...` | `--source-header` file missing |
| `Error: Unable to create output folder ...` / `Unable to write file ...` | file system errors |
| `Warning: ...` / `Note: ...` | printed before the result; the compile still succeeds |
