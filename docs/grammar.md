# Grammar language

A grammar file is a list of **rules** (statements). Each rule gives a name
to an expression; the expression says what input the rule matches. The
notation is a parsing expression grammar (PEG): alternatives are tried in
order, repetitions are greedy, and the same rule at the same position
always gives the same result.

The grammar of the grammar language itself, written in the language, is
[`input/Compiler.grammar`](../input/Compiler.grammar).

## Statements

```
Name = expression;          define a rule
Name |= expression;         Name = (old) | (expression);
Name &= expression;         Name = (old) & (expression);
@Name = expression;         define a rule and put it in the parser table
```

- A name is a letter or `_`, followed by letters, digits and `_`
  (`[_A-Za-z][_A-Za-z0-9]*`). Names are case sensitive.
- Each name is defined once with `=`. A second `=` is the error
  `Symbol A is defined more than once`.
- `|=` and `&=` extend a rule that is already defined above them; used
  before the `=` they are the error
  `Symbol A must be defined before using |=`.
- A rule can be used (referenced) before or after its definition; order
  does not matter, except for the parser table (below). Using a name that
  is never defined is an error.
- A rule that is not used by another rule and is not in the parser table
  gives the warning `symbol X is not used`.
- Every statement ends with `;`.

```
Keyword = "if";
Keyword |= "else";          // Keyword = "if" | "else";
Pair = Name;
Pair &= "=" & Value;        // Pair = Name & "=" & Value;
```

### Names become C++ identifiers

Each rule `Name` becomes the function `isName` and the enum value
`TokenType::Name` in the generated code. Therefore:

- `Unknown` is reserved: `TokenType::Unknown` (`0`) is the type of the
  root token. Using it as a rule name is the error
  `Symbol Unknown is reserved ...`.
- C++ keywords (`int`, `class`, `default`, `and`, ...) can not be rule
  names: `Symbol int is a C++ keyword`.
- Names that differ only by case are different rules.

### The parser table: `@`

A rule written with `@` in front of its name (in its `=`, `|=` or `&=`
statement) is a **start rule**. The generated `parserTable` lists the start
rules, **the last defined first**. At least one is required
(`No symbol is selected for the parser table, use @Symbol`).

The parser then works like this, until the input ends:

1. at the current position, try each start rule in table order;
2. the first one that matches **and consumes input** wins, its token is
   added to the result, and the loop continues after it;
3. if none consumes input, parsing stops.

The parse succeeds only if the whole input was consumed. Empty input is a
failure.

So the start rules are the things that can appear one after another at the
top level of the input. The grammar language itself has two:

```
@WhiteSpaceOrComment = (WhiteSpace | Comment)+;
@Statement = Symbol & WS & ...;
```

and its parser table is `{ isStatement, isWhiteSpaceOrComment, nullptr }`.

## Expressions

| Syntax | Name | Matches |
|--------|------|---------|
| `"text"` | literal | exactly these bytes |
| `""` | empty literal | nothing, always succeeds |
| `.` | any | one byte; fails only at the end of the input |
| `["x","y"]` | interval | bytes between `x` and `y` (see below) |
| `Name` | reference | the rule `Name` |
| `( e )` | group | `e` |
| `!e` | negation | succeeds when `e` fails; never consumes input |
| `e?` | zero or one | `e` if it matches, else nothing; always succeeds |
| `e*` | zero or more | `e` as many times as possible; always succeeds |
| `e+` | one or more | `e` at least once, then as many times as possible |
| `e{m}` | at least m | `e` at least `m` times, then as many as possible |
| `e{m,n}` | between | `e` at least `m` and at most `n` times (`n >= m`) |
| `a & b` | concatenation | `a` then `b` |
| `a \| b` | alternation | `a`; only if `a` fails, `b` |

### Precedence

From tightest to loosest:

1. **`!`** applies to the term right after it.
2. **Postfix** `?` `*` `+` `{m}` `{m,n}`: at most one per term. `A*?` is
   a syntax error, write `(A*)?` (which is simplified to `A*` anyway).
   Postfix applies to the whole negated term: `!"x"*` is `(!"x")*`; write
   `!("x"*)` to negate a repetition.
3. **`&`**
4. **`|`**

`A & B | C & D` is `(A & B) | (C & D)`. Use parentheses for anything else:
`A & (B | C) & D`.

### Literals

Literals are written as C string literals and are copied **as written**
into the generated C++ code, so they must be valid C++:

| Escape | Byte |
|--------|------|
| `\"` `\\` `\'` `\?` | the character |
| `\n` `\r` `\t` `\a` `\b` `\f` `\v` | control characters |
| `\ooo` | octal, one to three digits |
| `\xHH...` | hexadecimal |

As in C++, a hex escape takes **all** the hex digits that follow:
`"\x41BC"` is one (invalid) escape, not `"ABC"`. Use octal (`"\101BC"`) or
end the literal (`"\x41" & "BC"`).

**No NUL bytes**: the runtime compares literals as C strings, so a
literal or interval bound containing `\0` / `\x00` is rejected with the
error `String "..." contains a NUL byte`. Match a NUL byte with
`!["\x01","\xFF"] & .`.

Literals are byte strings. UTF-8 text in a UTF-8 grammar file is matched
byte by byte, which works as long as the input uses the same encoding.

### Intervals

`["x","y"]` matches as many bytes as `x` has, each byte between the byte at
the same position in `x` and in `y`, compared as **unsigned** values:

```
Digit = ["0","9"];
Lower = ["a","z"];
High  = ["\x80","\xFF"];                // any byte 128..255
Two   = ["\xC3\x80","\xC3\xBF"];        // \xC3 followed by \x80..\xBF
```

Both strings must be non-empty and of the same length (in bytes after the
escapes are decoded). A multi-byte interval is **per byte**, not a
lexicographic range: `["a0","b9"]` matches `a0`..`a9` and `b0`..`b9`, but
not `az`.

### Counts

`e{m}` is "at least `m`", not "exactly `m`". For exactly `m`, write
`e{m,m}`.

| Written | Same as |
|---------|---------|
| `e{0}` | `e*` |
| `e{1}` | `e+` |
| `e{0,1}` | `e?` |
| `e{1,1}` | `e` |
| `e{3,3}` | exactly three |

Numbers are decimal; leading zeros do not make them octal (`{010}` is ten).
The largest count is 1000000. `{3,2}` is an error.

### Whitespace and comments in a grammar file

Spaces, tabs, new lines and a UTF-8 byte order mark can appear between any
two elements. Comments:

```
// to the end of the line
#  to the end of the line
/* over
   several lines */
```

A `//` or `#` comment must end with a new line: a line comment on the last
line of a file without a final new line is a syntax error.

## How matching works

The generated parser reads the whole input into memory and calls the rule
functions recursively. Knowing these rules avoids most surprises:

- **Ordered choice.** In `A | B`, `B` is tried only if `A` fails. If `A`
  succeeds, `B` is never tried at that point, **even if what comes after the
  choice then fails**: with `X = "a"; Y = "a" & "b";`, the rule
  `S = (X | Y) & "c";` does not match `abc` — `X` matches `a`, `"c"` fails
  on `b`, and `Y` is not tried again. Put the longer alternative first
  (`Y | X`). For literals and for alternatives written in the same rule the
  compiler does this itself (see
  [Optimizations](transformations.md#rule-5--longest-match)).
- **Greedy repetition.** `*`, `+` and counts take as many as they can and
  never give back: `"a"* & "a"` never matches, and `.* & "!"` never matches
  (`.*` already ate the `!`). To read up to a terminator, exclude it in the
  loop: `(!"!" & .)* & "!"`.
- **Negation looks ahead.** `!e` tests `e` at the current position and
  restores the position; tokens produced while testing `e` are dropped.
  `!"*/" & .` is "any byte that does not start `*/`".
- **Loops that do not advance stop.** A `*`, `+` or count iteration that
  succeeds without consuming input ends the loop (the compiler also warns:
  `repeats an expression that can match empty input`).
- **A failed rule is remembered.** When rule `R` fails at position `p`, the
  parser remembers it and does not try `R` at `p` again. This keeps
  backtracking grammars fast. Rules that succeed are evaluated again when
  needed.
- **Nesting is limited.** Each rule call inside another uses one level of a
  budget of 1024; deeper input (for example 400 nested parentheses in the
  calculator grammar, three rules per level) makes the parser throw
  `std::runtime_error("parser-stack-overflow")`.

## Recipes

Whitespace and comments between top-level items — make them start rules:

```
NewLine = "\r\n" | "\n";
@Space = (" " | "\t" | NewLine)+;
@LineComment = "//" & (!NewLine & .)* & NewLine?;
@Item = ...;
```

Optional whitespace inside a rule:

```
WS = (" " | "\t" | "\r" | "\n")*;
Call = Name & WS & "(" & WS & Arguments? & WS & ")";
```

List with separators:

```
Arguments = Expression & (WS & "," & WS & Expression)*;
```

Identifier and keyword (a keyword must not continue as an identifier):

```
IdStart = "_" | ["a","z"] | ["A","Z"];
IdChar  = IdStart | ["0","9"];
Keyword = ("if" | "else" | "while") & !IdChar;
Name    = !Keyword & IdStart & IdChar*;
```

Quoted string with backslash escapes:

```
String = "\"" & (!"\"" & (("\\" & .) | .))* & "\"";
```

Case-insensitive word:

```
Select = ("s" | "S") & ("e" | "E") & ("l" | "L") & ("e" | "E") & ("c" | "C") & ("t" | "T");
```

Operator precedence — one rule per level, loosest at the top:

```
Sum     = Product & (WS & ("+" | "-") & WS & Product)*;
Product = Factor & (WS & ("*" | "/") & WS & Factor)*;
Factor  = Number | "(" & WS & Sum & WS & ")";
```

Left recursion can be written directly; it is rewritten to a loop:

```
Sum = Sum & "+" & Product | Product;    // => Sum = Product & ("+" & Product)*;
```

Fixed-size fields:

```
Hex  = ["0","9"] | ["a","f"] | ["A","F"];
Uuid = Hex{8,8} & "-" & Hex{4,4} & "-" & Hex{4,4} & "-" & Hex{4,4} & "-" & Hex{12,12};
```

One UTF-8 encoded character (not validated beyond the lead byte ranges,
NUL excluded, see literals):

```
Tail  = ["\x80","\xBF"];
Utf8  = ["\x01","\x7F"]
      | ["\xC2","\xDF"] & Tail
      | ["\xE0","\xEF"] & Tail{2,2}
      | ["\xF0","\xF4"] & Tail{3,3};
```

## Rules that shape the token tree

How you split the grammar into rules decides what the token tree looks
like, because every rule that matches becomes a node and groups do not:

- Make a rule for every piece you want to find in the tree (`Name`,
  `Number`, `Operator`).
- Text matched directly by a rule ends up in that rule's node; a rule that
  matches only text has it as its value (`Number [10]`).
- Groups, loops and `?` do not create nodes; what they match is added to
  the enclosing rule.

See [Runtime and token tree](runtime.md) for the details.
