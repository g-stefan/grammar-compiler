# Runtime and token tree

The generated code needs the runtime library `XYO::GrammarCompiler`
(`#include <XYO/GrammarCompiler.hpp>`, fabricare dependency
`"grammar-compiler"`). It has three classes: `Input` (the text), `Token`
(a node of the result) and `Parser` (runs a parser table over an input).

## Parsing

```cpp
#include <My/Code/ParserTable.hpp>
#include <My/Code/TokenType.hpp>

using namespace XYO::GrammarCompiler;

bool load(const char *fileName) {
	Parser parser;
	try {
		if (!parser.parseFile(fileName, My::Code::parserTable)) {
			printf("%s: syntax error, line %zu\n", fileName, parser.getErrorLine());
			return false;
		};
	} catch (const std::exception &e) {
		printf("%s: %s\n", fileName, e.what()); // parser-stack-overflow
		return false;
	};
	// parser.tokenList is the root of the token tree, valid while parser lives
	return process(parser.tokenList);
};
```

| Call | Result |
|------|--------|
| `parser.parseContent(text, table)` | parse a `String` (or `const char *`) |
| `parser.parseFile(fileName, table)` | read the whole file, then `parseContent`; `false` if the file can not be read |
| `parser.parse(table)` | parse the input already set with `parser.input.setContent(text)`, without clearing the failure memo |

All three return `true` only if the parser table consumed the **whole**
input (see [the parser table](grammar.md#the-parser-table-)). Empty input
returns `false`.

### Errors

- **Syntax error**: `false` is returned. `parser.getErrorLine()` is the
  line (1-based) of `parser.input.farthestPosition`, the farthest byte the
  parser read. It is usually where the input stops making sense: the
  parser got that far on some alternative before every way failed. For a
  column, count back from `farthestPosition` to the previous `'\n'` in
  `parser.input.buffer`.
- **Nesting too deep**: each rule call inside another rule uses one level
  of a budget of 1024 (the `level` argument). When it runs out, `codeBegin`
  throws `std::runtime_error("parser-stack-overflow")`. Inputs from
  untrusted sources can always trigger it, so wrap parsing in `try` /
  `catch`; nothing catches it for you (`XYO_APPLICATION_MAIN` does not).
- **Out of memory**: `std::bad_alloc` from the allocators.

On `false`, `parser.tokenList` still holds the tokens of the top-level
matches that succeeded before the error.

### One parser per input

`parseContent` resets the input and the failure memo but **not**
`tokenList`: parsing a second input with the same `Parser` appends its
tokens after those of the first. Use a new `Parser` for each input (it is
cheap: a root token and an empty memo).

### Choosing the start rule

The generated `parserTable` holds the `@` rules. A parser table is just a
`nullptr` terminated array of rule functions, so you can make your own to
start from any rule:

```cpp
ParserProcessToken expressionTable[] = {
	My::Code::isWS,
	My::Code::isSum,
	nullptr};

Parser parser;
bool ok = parser.parseContent("1 + 2 * 3", expressionTable);
```

The functions are tried in array order at each position; the first one
that consumes input wins (rules that match nothing are skipped).

### Memory initialization

`Token` objects come from an active memory pool of `xyo-managed-memory`.
As in every XYO program, initialize the pool before the objects that use
it, from your application's static `initMemory`:

```cpp
static void initMemory() {
	Parser::initMemory();   // String and Token pools
};
```

`XYO_APPLICATION_MAIN` calls `Application::initMemory()` when it exists.

## The token tree

`parser.tokenList` is the **root** token: type `0` (`Unknown`), empty
value. Its children are the tokens of the top-level matches, in input
order. Each token is a `TXList5Node` with `parent`, `childHead`,
`childTail`, `back` and `next` pointers, plus:

| Field | Meaning |
|-------|---------|
| `type` | the rule that made it: `My::Code::TokenType::Rule` (`Token::Type` is `uint16_t`) |
| `value` | text (see below); empty for tokens with children |
| `subType` | always `0`; free for your own use while processing |

How tokens are made — with the calculator of
[Getting started](getting-started.md#2-a-first-parser), input
`x = 2 * (y + 10);`:

```
Assignment []               <- @Assignment, a child of the root
    WS []                   <- a rule that matched nothing still has a token
    Name [x]                <- a rule that matched only text: the text is its value
    WS [ ]
    Assignment [=]          <- text matched by Assignment itself: a child of type Assignment
    WS [ ]
    Sum []
        Product []
            Factor []
                Number [2]
            WS [ ]
            Mul [*]
            WS [ ]
            Factor []
                Factor [(]  <- "(" matched by Factor
                WS []
                Sum []
                    ...
                WS []
                Factor [)]
    WS []
    Assignment [;]
    WS []
```

1. **Every rule that matches adds one token** of its type to the token of
   the rule that called it (or to the root, for a top-level match). Rules
   that fail add nothing.
2. **Text matched directly by a rule** (literals, intervals, `.`) is added
   to that rule's token as children **of the same type**; consecutive
   pieces are merged into one child.
3. **A rule whose token ends up with a single such text child** keeps the
   text as its own `value` and has no children (`Name [x]`,
   `Number [10]`). This is the common case of "lexical" rules.
4. **Groups, `?`, `*`, `+`, counts** create no tokens: what they match is
   added to the enclosing rule's token in order.
5. **`!e`** never leaves tokens.
6. **Rules that match nothing** (`WS = " "*;` on no spaces) still add a
   token, with an empty value.

The text of the input matched by any token is the concatenation of the
`value` of the token and all its descendants, in order. Nothing is lost:
concatenating every value under the root gives back the parsed input.

### Shaping the tree from the grammar

- Want to find something in the tree? Give it its own rule.
- Want a token to carry its text directly? Make the rule match only text
  (no references to other rules): `Number = ["0","9"]+;`.
- Don't want noise tokens? You can not suppress them in the grammar; skip
  their types while walking (`WS`, punctuation tokens of the parent type).

### Walking the tree

Recursively over the children:

```cpp
using namespace My::Code;

void walk(Token *token) {
	Token *scan;
	for (scan = token->childHead; scan != nullptr; scan = scan->next) {
		switch (scan->type) {
		case TokenType::WS:
			continue;
		case TokenType::Number:
			printf("number %s\n", scan->value.value());
			continue;
		default:
			break;
		};
		walk(scan);
	};
};

walk(parser.tokenList);
```

In pre-order without recursion, with the `TXList5` helpers
(`TokenList::begin` returns the root):

```cpp
Token *scan;
for (scan = TokenList::begin(parser.tokenList); scan != nullptr; scan = TokenList::successor(scan)) {
	printf("%s [%s]\n", getTokenTypeString(scan->type), scan->value.value());
};
```

`TokenList::successorNoChild(token)` skips the children of `token`.

The full text of a token:

```cpp
String tokenText(Token *token) {
	String retV = token->value;
	Token *scan;
	for (scan = token->childHead; scan != nullptr; scan = scan->next) {
		retV += tokenText(scan);
	};
	return retV;
};
```

### Changing the tree

The tree is a `TXList5<Token, TMemoryPoolActive>` (`TokenList`), so the
`TXList5` functions work on it: `TokenList::newNode()`,
`TokenList::pushToTail(parent, node)`, `TokenList::extract(parent, node)`,
`TokenList::destructor(node)` (frees the node and its children),
`Parser::tokenTransferChild(to, from)`. Tokens are freed with the parser;
a token extracted from the tree must be freed with
`TokenList::destructor`.

## Performance

- The whole input is in memory; backtracking just resets a position.
- A rule that **fails** at a position is remembered (a hash set of
  `(position, rule)` pairs, `ParserMemoization`) and not tried there again.
  With the compiler's prefix factoring this keeps typical grammars close to
  linear. Successful matches are not cached: a rule that succeeds and is
  then backtracked over runs again if needed.
- Tokens come from a memory pool and are reused.

## Threads

A `Parser`, its input and its tokens belong to the thread that created
them: the XYO memory pools are per thread. Parse on as many threads as you
like, each with its own `Parser`; to hand results to another thread, copy
the data out of the tokens (strings, numbers), not the tokens. The
generated code has no state of its own besides the `parserTable` array.
