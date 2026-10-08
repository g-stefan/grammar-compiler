# API reference

Runtime library: `#include <XYO/GrammarCompiler.hpp>`, namespace
`XYO::GrammarCompiler`, fabricare dependency `"grammar-compiler"`.

## Namespaces and macros

| Symbol | Meaning |
|--------|---------|
| `XYO::GrammarCompiler` | the runtime; contains `using namespace` of `XYO::ManagedMemory`, `XYO::DataStructures`, `XYO::Encoding`, `XYO::System` |
| `XYO_GRAMMARCOMPILER_EXPORT` | DLL export / import of the runtime functions |
| `XYO_GRAMMARCOMPILER_LIBRARY` | define when compiling the runtime into your program (static, no DLL) |
| `XYO_GRAMMARCOMPILER_INTERNAL`, `XYO_GRAMMARCOMPILER_DLL_INTERNAL` | set by the build of the library itself |
| `XYO::GrammarCompiler::Version::version()`, `build()`, `versionWithBuild()`, `datetime()` | library version |
| `XYO::GrammarCompiler::Copyright::copyright()`, `publisher()`, `company()`, `contact()` | copyright |
| `XYO::GrammarCompiler::License::license()`, `shortLicense()` | license text (`std::string`) |

## `Input`

The whole input in memory. Not copyable.

| Member | Meaning |
|--------|---------|
| `String content` | the input text |
| `const char *buffer` | `content.value()` |
| `size_t size` | length in bytes |
| `size_t position` | current position |
| `size_t farthestPosition` | farthest position reached while parsing, for error reporting |
| `char input` / `value()` / `operator char()` | byte at `position`, `0` at the end |
| `void setContent(const String &value)` | set the text, position and farthest position to 0 |
| `bool read()` | advance one byte; `false` at the end |
| `bool isEof()` | `position >= size` |
| `void setPosition(size_t value)` | move (backtrack) without updating `farthestPosition` |
| `void skip(size_t length)` | advance `length` bytes (must not exceed the rest) |
| `size_t getLineNumber(size_t position)` | 1-based line of a position (counts `'\n'`) |

## `Token`

```cpp
struct Token : public TXList5Node<Token> {
	typedef uint16_t Type;
	static const uint16_t TypeUnknown = 0;

	Type type;
	Type subType;
	String value;
};
typedef TXList5<Token, TMemoryPoolActive> TokenList;
```

| Member | Meaning |
|--------|---------|
| `type` | `TokenType::Rule` of the generated code; `0` for the root |
| `subType` | `0`, not used by the parser; free for the application |
| `value` | matched text of a leaf token |
| `parent`, `childHead`, `childTail`, `back`, `next` | tree links (from `TXList5Node`) |
| `TokenList::newNode()` | new token from the pool |
| `TokenList::destructor(token)` | free a token and its children |
| `TokenList::pushToTail(parent, token)`, `push`, `extract(parent, token)`, `pop`, `popFromTail`, `insertNode`, `transferChild` | tree editing |
| `TokenList::begin(token)` | the root of the tree containing `token` |
| `TokenList::successor(token)` / `successorNoChild(token)` / `predecessor(token)` / `end(token)` | pre-order traversal |

See [Runtime and token tree](runtime.md#the-token-tree) for how the parser
builds the tree.

## `ParserProcessToken`

```cpp
typedef bool (*ParserProcessToken)(Parser &parser, Token *token, int level);
```

The signature of every generated rule function `isRule`: try to match the
rule at the current position; on success add its token as the last child
of `token`, advance the input and return `true`; on failure leave the input
and `token` unchanged and return `false`. `level` is the remaining nesting
budget (start with `1024`). A **parser table** is a `nullptr` terminated
array of these.

## `Parser`

Not copyable.

| Member | Meaning |
|--------|---------|
| `Token *tokenList` | root of the token tree, owned by the parser |
| `Input input` | the input |
| `ParserMemoization memoization` | failed `(position, rule)` pairs |
| `bool parseContent(const String &content, ParserProcessToken *parseTable)` | clear the memo, set the input, `parse` |
| `bool parseFile(const char *fileName, ParserProcessToken *parseTable)` | read the file, `parseContent`; `false` if unreadable |
| `bool parse(ParserProcessToken *parseTable)` | repeat: first table function that consumes input wins; `true` if the whole input was consumed; `false` for empty input |
| `size_t getErrorLine()` | line of `input.farthestPosition` |
| `static void initMemory()` | initialize the `Token` (and `String`) pools |
| `size_t inputPosition()` / `void inputRestore(size_t position)` | save / restore the position |

Throws `std::runtime_error("parser-stack-overflow")` when the nesting
budget runs out. `parseContent` / `parseFile` do not clear `tokenList`.

Functions used by the generated code (and usable in hand-written rule
functions with the same signature):

| Member | Meaning |
|--------|---------|
| `bool codeBegin(Token *&node, Token::Type type, int &level, size_t &position)` | start of a rule: `false` if the rule is known to fail here; decrement `level` (throws at 0); create the rule's token |
| `void codeEndTrue(Token *token, Token *node)` | success: merge text children (`tokenChildToValue`), add `node` to `token` |
| `void codeEndFalse(Token *node, Token::Type type, size_t position)` | failure: restore the position, free `node`, remember the failure |
| `bool inputIs(const char *value)` | match and skip a literal, no token |
| `bool inputIsToken(const char *value, Token::Type type, Token *token)` | match a literal, add its text to `token` |
| `bool inputIsBetween(const char *a, const char *b, Token::Type type, Token *token)` | match an interval (`strlen(a)` bytes, each between the bytes of `a` and `b`, unsigned) |
| `bool inputIsTokenAny(Token::Type type, Token *token)` | match any one byte |
| `bool inputIsTokenThenSet(ParserProcessToken fn, Token::Type type, Token *token, int level)` | run `fn` into a new token of `type`; add it on success |
| `bool inputIsOneOrMoreToken(const char *value, Token::Type type, Token *token)` | one or more repetitions of a literal, as one token |
| `bool inputIsZeroOrMoreTokenValue(const char *value, Token::Type type, Token *token)` | append repetitions of a literal to `token->value`; always `true` |
| `static void tokenAddValue(Token *token, Token::Type type, const char *value, size_t length)` | add text to `token`, merged into the last child when it is a text child of the same type |
| `static void tokenChildToValue(Token *token)` | merge consecutive text children of the token's type; a single remaining one becomes the token's `value` |
| `static void tokenTransferChild(Token *token, Token *node)` | move all children of `node` to the end of `token` |

## `ParserMemoization`

Open addressing hash set of `(position, type)` pairs. Not copyable.

| Member | Meaning |
|--------|---------|
| `bool has(size_t position, Token::Type type)` | the rule failed here before |
| `void set(size_t position, Token::Type type)` | remember a failure |
| `void empty()` | forget everything, free the table |
| `table`, `capacity`, `count` | storage (capacity is a power of two, grows at half full) |

## Generated code

For `--source-namespace=N`:

| Symbol | Header | Meaning |
|--------|--------|---------|
| `N::TokenType::Unknown` | `TokenType.hpp` | `0` |
| `N::TokenType::Rule` | `TokenType.hpp` | one per rule, `1, 2, ...` in definition order |
| `const char *N::getTokenTypeString(Token::Type type)` | `TokenType.hpp` | rule name, `"Unknown"` otherwise |
| `bool N::isRule(Parser &parser, Token *token, int level)` | `Code.hpp` | one per rule |
| `ParserProcessToken N::parserTable[]` | `ParserTable.hpp` | the `@` rules, last defined first, `nullptr` terminated |

The enum values change when rules are added, removed or reordered:
compare with the names, never store the numbers.

## Compiler application

`#include <XYO/GrammarCompiler.Application.hpp>` with the static library
`grammar-compiler.application.static` gives the compiler without its
`main`:

```cpp
XYO::GrammarCompiler::Application::Application compiler;
int exitCode = compiler.main(cmdN, cmdS); // same arguments as the executable, cmdS[0] ignored
```

`Application::initMemory()` initializes the pools it needs. The other
internal namespaces (`Grammar`, `Transformer`, `Generator`, `Debug`,
`Code`) are implementation details and may change.
