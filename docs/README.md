# Grammar Compiler — Documentation

`grammar-compiler` turns a **grammar file** into **C++ source code for a
parser**. You describe a language (a configuration format, a small script
language, a command syntax, the grammar language itself ...) with rules like

```
Number = ["0","9"]+;
Sum    = Number & ("+" & Number)*;
@Line  = Sum & "\n";
```

and the compiler writes one C++ function per rule (`isNumber`, `isSum`,
`isLine`), a `TokenType` enum with one value per rule, and a `parserTable`
of the start rules. Linked with the small runtime library
`XYO::GrammarCompiler` (`Parser`, `Input`, `Token`), the generated code
reads a file or a string and builds a **token tree**: one node for each rule
that matched, holding the matched text.

The project has two parts:

- **The compiler** (`grammar-compiler` executable): reads a `.grammar` file,
  checks it, optimizes it, and writes the parser source code.
- **The runtime library** (`grammar-compiler` library, namespace
  `XYO::GrammarCompiler`, header `<XYO/GrammarCompiler.hpp>`): the parser
  engine the generated code calls — input buffer, backtracking, failure
  memoization, token tree.

The grammar compiler is written with itself: its own grammar is
[`input/Compiler.grammar`](../input/Compiler.grammar) and the parser that
reads `.grammar` files is generated code, in
`source/XYO/GrammarCompiler.Application/Code/`.

```
your program
generated parser      is<Rule>.cpp, TokenType, ParserTable  <-- grammar-compiler writes these
grammar-compiler      Parser, Input, Token                  <-- runtime library (this repository)
xyo-system            (Shell, File, IApplication)
xyo-encoding          (String)
xyo-multithreading
xyo-data-structures   (TXList5 token tree, TRedBlackTree)
xyo-managed-memory    (memory pools)
xyo-platform
```

## Why it exists

Hand-written recursive descent parsers are easy to start and hard to keep
correct; general parser generators bring their own runtime, build steps and
memory model. `grammar-compiler` sits in between: the output is plain,
readable C++ (one small function per rule, the rule as a comment above it),
built like any other source of an XYO project, and the grammar is written
in a compact PEG-like notation.

| Problem | What `grammar-compiler` does |
|---------|------------------------------|
| Backtracking parsers can take exponential time | failures are memoized per (position, rule); common prefixes of alternatives are factored out |
| Left recursive rules (`E = E & "+" & N \| N`) loop forever in a top-down parser | direct left recursion is rewritten to a loop (`E = N & ("+" & N)*`); indirect left recursion is reported as an error |
| `"a" \| "ab"` never matches `ab` in an ordered choice | the shorter alternative is moved last (`"a" & "b"?`), and a note is printed |
| Alternatives that can never be reached hide grammar mistakes | they are removed with a warning |
| A loop over something that can match nothing hangs | every loop stops when an iteration consumes no input; a warning is printed |
| Deep input nesting overflows the C++ stack | the rule nesting depth is limited, the parser throws `parser-stack-overflow` |
| Where did the input stop matching? | `Parser::getErrorLine()` gives the line of the farthest position the parser read |

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Define a rule | `Name = expression;` | one definition per name |
| Extend a rule | `Name \|= expression;` / `Name &= expression;` | `(old) \| (new)` / `(old) & (new)` |
| Start rule(s) | `@Name = expression;` | goes in `parserTable`; the parser repeats them until the input ends |
| Text, any byte, byte range | `"text"`, `.`, `["a","z"]` | strings are C string literals, bytes compared unsigned |
| Sequence, ordered choice | `A & B`, `A \| B` | `&` binds tighter than `\|`; first alternative that matches wins |
| Repetition | `A?`, `A*`, `A+`, `A{m}`, `A{m,n}` | greedy, never gives back input |
| Not followed by | `!A` | consumes nothing |
| Generate code | `grammar-compiler --grammar=g --source-header=h --source-namespace=N ...` | or `@options-file` |
| See the optimized grammar | `grammar-compiler --grammar=g --output-grammar=out` | no code is generated |
| Parse | `Parser parser; parser.parseFile(name, N::parserTable)` | `true` when the whole input matched |
| Read the result | `parser.tokenList` children | `token->type` is `N::TokenType::Rule`, `token->value` the text |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build and install, a complete first parser from grammar to running program, project layout, building without fabricare |
| [Grammar language](grammar.md) | Every operator, precedence, strings and escapes, intervals, counts, `@`, `\|=`, `&=`, comments, PEG matching rules, pitfalls |
| [Command line](command-line.md) | All options, options files, which files are generated and what they contain, messages and exit codes |
| [Optimizations and diagnostics](transformations.md) | The grammar transformations, notes, warnings and errors, how to read `--output-grammar` |
| [Runtime and token tree](runtime.md) | `Parser`, `Input`, `Token`, the shape of the token tree, walking it, errors, limits, threads |
| [API reference](reference.md) | Every public symbol of the runtime library and of the generated code |

The types used by the runtime come from the XYO layers below it: `String`
from `xyo-encoding`, `TXList5` / `TXList5Node` and `TMemoryPoolActive`
from `xyo-data-structures` / `xyo-managed-memory`, `Shell` and
`IApplication` from `xyo-system`. Each of those repositories has its own
`docs/`.

## Source map

```
source/XYO/GrammarCompiler.hpp                umbrella header of the runtime library, include this
source/XYO/GrammarCompiler/
    Dependency.hpp                            xyo-system, export macro, namespaces
    Input[.cpp]                               input buffer, position, farthest position, line numbers
    Token[.cpp]                               Token (tree node), TokenList
    Parser[.cpp]                              Parser, ParserMemoization, ParserProcessToken
    Copyright / License / Version             library metadata
    Library.rc / *.rh                         Windows DLL version resource
source/XYO/GrammarCompiler.Amalgam.cpp        the runtime library as one file
source/XYO/GrammarCompiler.Application/
    Application[.cpp]                         command line, the compile pipeline
    Grammar[.cpp]                             GrammarNode tree, build from tokens, validate, print
    Transformer[.cpp]                         optimizations, left recursion, warnings
    Generator[.cpp]                           writes the C++ files
    Debug[.cpp]                               token tree printing, for development
    Code/                                     parser of .grammar files, generated from input/Compiler.grammar
source/XYO/GrammarCompiler.Application.Amalgam.cpp   the compiler as one file
input/Compiler.grammar                        the grammar of the grammar language
input/Compiler.not-optimized.grammar          the same, written without optimizations
input/Grammar.transform.txt                   the transformation rules
input/Code.Source.Header.hpp                  header text of the generated Code/ files
input/Code.Generate.arguments                 options used by fabricare generate
fabricare/generate.js                         fabricare generate: regenerate Code/ with the built compiler
fabricare/test.js                             fabricare test: self-hosting, transformations, errors, parsers
test/Transform/*.grammar / *.expected         optimization tests
test/Error/*.grammar                          grammars that must be rejected
test/Parser/*.grammar, Test.Parser.cpp        generated parsers run on sample inputs
```

## AI assistant skill

A Claude Code skill describing how to use the grammar compiler and its
runtime lives in
[`.claude/skills/grammar-compiler/`](../.claude/skills/grammar-compiler/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that use
`grammar-compiler`.
