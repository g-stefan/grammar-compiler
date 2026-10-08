# Getting started

## 1. Build and install

The project is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool used by all XYO C++ projects. The XYO layers up to
`xyo-system` must be installed to the SDK first. From the repository root:

```bash
fabricare make       # build into output/
fabricare test       # run the test suite (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
fabricare generate   # regenerate the compiler's own parser from input/Compiler.grammar
```

| Project | Kind | Notes |
|---------|------|-------|
| `grammar-compiler` (library) | DLL / shared library (`dll-or-lib`) | the runtime `XYO::GrammarCompiler`; a static library when the platform is built static |
| `grammar-compiler.application.static` | static library | the compiler without `main` (`XYO_GRAMMARCOMPILER_APPLICATION_LIBRARY`), to embed it in another tool |
| `grammar-compiler` (executable) | command line tool | the compiler |

After `fabricare install` the executable is in `~/.fabricare/<platform>/bin`
and the headers and libraries in `include/` and `lib/` next to it, so other
fabricare projects can depend on `"grammar-compiler"`.

`fabricare test` checks that:

- the parser generated from `input/Compiler.grammar` is the one in
  `source/XYO/GrammarCompiler.Application/Code/` (else: run
  `fabricare generate`), and that a compiler rebuilt from it generates the
  same code again (self-hosting);
- each grammar in `test/Transform/` is optimized to its `.expected` file,
  and optimizing the result again changes nothing;
- `input/Compiler.not-optimized.grammar` is optimized to the same grammar as
  `input/Compiler.grammar`;
- each grammar in `test/Error/` is rejected;
- the parsers generated from `test/Parser/*.grammar` accept and reject the
  inputs in `test/Parser/Test.Parser.cpp`.

## 2. A first parser

A tiny language of assignments, `x = 2 * (y + 10);`.

### The grammar

`input/Calc.grammar`:

```
// Calculator assignments
WS = (" " | "\t" | "\r" | "\n")*;
Number = ["0","9"]+;
Name = ("_" | ["a","z"] | ["A","Z"]) & ("_" | ["a","z"] | ["A","Z"] | ["0","9"])*;
Add = "+" | "-";
Mul = "*" | "/";
Factor = Number | Name | "(" & WS & Sum & WS & ")";
Product = Factor & (WS & Mul & WS & Factor)*;
Sum = Product & (WS & Add & WS & Product)*;
@Assignment = WS & Name & WS & "=" & WS & Sum & WS & ";" & WS;
```

`@Assignment` is the start rule: the parser matches it again and again
until the input ends. See [Grammar language](grammar.md) for the notation.

### The source header

Every generated file starts with the text of a header file, usually the
license comment of your project. `input/Calc.Source.Header.hpp`:

```cpp
// Calc
// Copyright (c) 2026 Your Name
// SPDX-License-Identifier: MIT
```

### Generate the code

`input/Calc.arguments` (an options file, one option per line):

```
--grammar=../input/Calc.grammar
--source-header=../input/Calc.Source.Header.hpp
--source-namespace="Calc::Code"
--source-include-guard="CALC_CODE"
--source-include-path="Calc/Code"
--output-folder="Calc/Code"
```

Run it from the `source/` folder, so that `--output-folder` and
`--source-include-path` name the same folder:

```bash
cd source && grammar-compiler @../input/Calc.arguments
```

This writes `source/Calc/Code/`:

```
Code.hpp            declarations of isWS, isNumber, ... isAssignment
is<Rule>.cpp        one file per rule
TokenType.hpp/.cpp  enum Calc::Code::TokenType { Unknown, WS, Number, ... }, getTokenTypeString
ParserTable.hpp/.cpp  Calc::Code::parserTable = { isAssignment, nullptr }
Code.Amalgam.cpp    all the .cpp files above in one (fabricare skips it)
```

To see what the compiler made of the grammar before generating code:

```bash
grammar-compiler --grammar=input/Calc.grammar --output-grammar=Calc.optimized.grammar
```

### The program

`source/Calc/Main.cpp`:

```cpp
#include <Calc/Code/ParserTable.hpp>
#include <Calc/Code/TokenType.hpp>

using namespace XYO::GrammarCompiler;

class Application : public virtual IApplication {
		XYO_PLATFORM_DISALLOW_COPY_ASSIGN_MOVE(Application);

	public:
		inline Application(){};

		static void initMemory() {
			Parser::initMemory();
		};

		static void print(Token *token, int depth) {
			Token *scan;
			for (scan = token->childHead; scan != nullptr; scan = scan->next) {
				printf("%*s%s [%s]\n", depth * 4, "", Calc::Code::getTokenTypeString(scan->type), scan->value.value());
				print(scan, depth + 1);
			};
		};

		int main(int cmdN, char *cmdS[]) {
			Parser parser;
			try {
				if (!parser.parseContent(cmdN > 1 ? cmdS[1] : "x = 2 * (y + 10);", Calc::Code::parserTable)) {
					printf("Syntax error, line %zu\n", parser.getErrorLine());
					return 1;
				};
			} catch (const std::exception &e) {
				printf("Error: %s\n", e.what());
				return 1;
			};
			print(parser.tokenList, 0);
			return 0;
		};
};

XYO_APPLICATION_MAIN(Application);
```

`fabricare.json`:

```json
{
	"solution": {
		"name": "calc",
		"type": "xyo-cpp",
		"projects": [
			{
				"name": "calc",
				"make": "exe",
				"sourcePath": [
					"Calc",
					"Calc/Code"
				],
				"dependency": [
					"grammar-compiler"
				]
			}
		]
	}
}
```

`fabricare make`, then `output/bin/calc` prints:

```
Assignment []
    WS []
    Name [x]
    WS [ ]
    Assignment [=]
    WS [ ]
    Sum []
        Product []
            Factor []
                Number [2]
            WS [ ]
            Mul [*]
            WS [ ]
            Factor []
                Factor [(]
                WS []
                Sum []
                    Product []
                        Factor []
                            Name [y]
                    WS [ ]
                    Add [+]
                    WS [ ]
                    Product []
                        Factor []
                            Number [10]
                WS []
                Factor [)]
    WS []
    Assignment [;]
    WS []
```

Read it with [Runtime and token tree](runtime.md): every rule that matched
is a node; text matched directly by a rule (`"="`, `"("`) is a child of the
rule's own type; a rule whose match is only text keeps it in its own value
(`Number [2]`); a rule that matched nothing still has a node (`WS []`).

With the two line input `a = 1;` / `b = (2 +;` it prints
`Syntax error, line 2`.

## 3. Keep the generated code up to date

Two ways, both used in XYO projects:

- **Commit the generated code** (what this repository does with its own
  parser). Add `fabricare/generate.js`:

  ```js
  messageAction("generate");

  runInPath("source", function() {
  	exitIf(Shell.system("grammar-compiler @../input/Calc.arguments"));
  });
  ```

  and run `fabricare generate` after changing the grammar. The build does
  not need `grammar-compiler` installed.

- **Generate on every build**: put the same code in
  `fabricare/make.prepare.js`, which `fabricare make` runs first. The
  grammar is then the only source of truth, but `grammar-compiler` must be
  on `PATH` wherever the project is built.

Either way, add the output folder to `sourcePath` and `"grammar-compiler"`
to `dependency`. Since the generated files are recreated from the grammar,
do not edit them by hand.

## 4. Include and namespaces

```cpp
#include <XYO/GrammarCompiler.hpp>
```

pulls in `<XYO/System.hpp>` and the runtime (`Input`, `Token`, `Parser`).
The generated `TokenType.hpp` and `ParserTable.hpp` include it already.
Namespace `XYO::GrammarCompiler` contains

```cpp
using namespace XYO::ManagedMemory;
using namespace XYO::DataStructures;
using namespace XYO::Encoding;
using namespace XYO::System;
```

so `using namespace XYO::GrammarCompiler;` is enough to write `String`,
`Shell`, `IApplication`, `Parser`, `Token` unqualified. The generated code
lives in the namespace given by `--source-namespace` (here `Calc::Code`),
which also has `using namespace XYO::GrammarCompiler;`. Two grammars in the
same program need two namespaces and two include guard prefixes.

The metadata namespaces exist in every XYO layer: write
`XYO::GrammarCompiler::Version::version()` in full.

## 5. Building without fabricare

Compile the runtime amalgam and the generated amalgam with your sources,
together with the amalgams of the XYO layers below (see the `xyo-system`
getting started page for those):

1. Put `source/` of `xyo-platform`, `xyo-managed-memory`,
   `xyo-data-structures`, `xyo-multithreading`, `xyo-encoding`,
   `xyo-system` and `grammar-compiler` on the include path, plus the folder
   that contains the generated code folder (`--source-include-path` is
   relative to it).
2. Provide the configuration headers of the lower layers (see the
   `xyo-system` getting started page).
3. Define `XYO_GRAMMARCOMPILER_LIBRARY` and the `..._LIBRARY` macros of
   every layer below everywhere (plain static linking).
4. Link `pthread` on Linux.

Example on Linux, with the repositories side by side and the generated code
in `myproject/source/Calc/Code`:

```bash
g++ -std=c++17 \
    -Ixyo-platform/source -Ixyo-managed-memory/source \
    -Ixyo-data-structures/source -Ixyo-multithreading/source \
    -Ixyo-encoding/source -Ixyo-system/source \
    -Igrammar-compiler/source -Imyproject/source \
    -DXYO_PLATFORM_LIBRARY -DXYO_MANAGEDMEMORY_LIBRARY \
    -DXYO_DATASTRUCTURES_LIBRARY -DXYO_MULTITHREADING_LIBRARY \
    -DXYO_ENCODING_LIBRARY -DXYO_SYSTEM_LIBRARY \
    -DXYO_GRAMMARCOMPILER_LIBRARY \
    xyo-platform/source/XYO/Platform.Amalgam.cpp \
    xyo-managed-memory/source/XYO/ManagedMemory.Amalgam.cpp \
    xyo-data-structures/source/XYO/DataStructures.Amalgam.cpp \
    xyo-multithreading/source/XYO/Multithreading.Amalgam.cpp \
    xyo-encoding/source/XYO/Encoding.Amalgam.cpp \
    xyo-system/source/XYO/System.Amalgam.cpp \
    grammar-compiler/source/XYO/GrammarCompiler.Amalgam.cpp \
    myproject/source/Calc/Code/Code.Amalgam.cpp \
    myproject/source/Calc/Main.cpp -o calc -pthread
```

On Windows with MSVC also define `XYO_PLATFORM_COMPILE_STATIC`.

Compile either `Code.Amalgam.cpp` or the individual generated `.cpp`
files, not both.
