# Grammar Compiler

Grammar Compiler to C++ code
- Describe a language in a compact PEG-like grammar: literals, byte intervals, any byte,
groups, negation, `?` `*` `+` `{m,n}`, sequence `&`, ordered choice `|`.
- Get plain C++ source: one function per rule, a `TokenType` enum, a parser table.
- The runtime library `XYO::GrammarCompiler` (`Parser`, `Input`, `Token`) parses a file or a
string into a token tree, with backtracking, failure memoization and error lines.
- The grammar is checked and optimized: common prefixes factored, direct left recursion
rewritten to loops, longest match notes, unreachable alternatives reported.

Self-hosted: the parser of `.grammar` files is generated from
[input/Compiler.grammar](input/Compiler.grammar). Built on `xyo-system`.

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, a first parser from grammar to program
- [Grammar language](docs/grammar.md) - operators, precedence, matching rules, recipes
- [Command line](docs/command-line.md) - options, options files, generated files, messages
- [Optimizations and diagnostics](docs/transformations.md) - transformations, warnings, errors
- [Runtime and token tree](docs/runtime.md) - `Parser`, `Token`, walking the tree, limits
- [API reference](docs/reference.md)

A Claude Code skill for this project is in
[.claude/skills/grammar-compiler](.claude/skills/grammar-compiler/SKILL.md).

## License

Copyright (c) 2022-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
