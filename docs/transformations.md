# Optimizations and diagnostics

Before generating code, the compiler checks the grammar and rewrites it
into an equivalent grammar that the generated top-down parser handles well.
You can write the grammar the natural way and let the compiler factor it:
[`input/Compiler.not-optimized.grammar`](../input/Compiler.not-optimized.grammar)
is the grammar language written without any optimization, and the test
suite checks that it compiles to the same grammar as the hand-optimized
[`input/Compiler.grammar`](../input/Compiler.grammar).

The rules are summarized in
[`input/Grammar.transform.txt`](../input/Grammar.transform.txt).

## See what the compiler did

```bash
grammar-compiler --grammar=my.grammar --output-grammar=my.optimized.grammar
```

writes the grammar after all transformations, one rule per line, and
prints the warnings and notes. The output is itself a valid grammar, and
compiling it again gives the same output. In the generated code, each rule
function has the rule as written, then `// Optimized: ...` and
`// Note: ...` comments when it was changed.

## The pipeline

1. **Parse** the grammar file (syntax errors).
2. **Build** the rules: apply `|=` / `&=`, check names, intervals, counts.
3. **Validate**: at least one `@` rule, every referenced rule exists; warn
   about rules that are never used.
4. **Simplify** each rule: rules 1, 2, 4, 5 below, duplicate and shadowed
   alternatives.
5. **Remove direct left recursion** (rule 3).
6. **Check indirect left recursion** (error).
7. **Remove unreachable alternatives**, simplify again, and **warn about
   loops** over expressions that can match nothing.

## Transformations

The choice operator is ordered (`A | B` tries `B` only if `A` fails) and an
expression always gives the same result at the same position. The rules
below keep that meaning, except rule 5, which is reported with a note.

### Rules 1 and 2 — common prefix

```
E = A & B | A;          =>  E = A & B?;
E = A & B | A & C;      =>  E = A & (B | C);
```

Without factoring, `A` is parsed again for every alternative, and for
nested rules the time grows exponentially (each group level of the
not-optimized grammar language would be about ten times slower). Works for
any number of alternatives and longer prefixes:

```
E = X & A & B | X & A & C | X | Y;   =>  E = X & (A & (B | C))? | Y;
```

Only **neighbouring** alternatives are merged: in `A & B | C | A & D`, the
order of `C` and `A & D` is the priority, so they are not moved.

### Rule 3 — left recursion

```
E = E & A | B;          =>  E = B & A*;
E = E & A1 | E & A2 | B1 | B2;   =>  E = (B1 | B2) & (A1 | A2)*;
```

`E = E & A` would call `E` again at the same position forever. Direct left
recursion is rewritten for any number of alternatives, and a note is
printed:

```
Note: symbol E: left recursion, E & "+" & N | N => N & ("+" & N)*
```

What is **not** rewritten, and is an error:

- indirect left recursion, `A = B & "a" | "x"; B = A & "b";` →
  `Left recursion A -> B -> A`;
- left recursion behind something that can match nothing,
  `A = WS & A & "x" | "y";` with `WS = " "*;` → `Left recursion A -> A`;
- a left recursive rule with no other alternative, `E = E & "a";` →
  `Symbol E is left recursive and has no alternative to start with`.

An alternative that is only the rule itself (`A = A | "y";`) is dropped
with the warning `alternative A has no effect`.

### Rule 4 — repetitions

| Written | Becomes |
|---------|---------|
| `(A?)?` | `A?` |
| `(A*)?`, `(A+)?` | `A*` |
| `(A?)*`, `(A*)*`, `(A+)*` | `A*` |
| `(A?)+`, `(A*)+` | `A*` |
| `(A+)+` | `A+` |
| `A{0,1}` | `A?` |
| `A{1,1}` | `A` |
| `A{0}` | `A*` |
| `A{1}` | `A+` |

### Rule 5 — longest match

```
E = A | A & B;          =>  E = A & B?;
E = "a" | "ab" | "abc"; =>  E = "a" & ("b" & "c"?)?;
```

Here the meaning **changes**: as written, `A | A & B` always matches `A`
and never tries `A & B`. The intent is almost always to match the longer
form when possible, so the shorter alternative is moved last. The same is
done for literals where one is the beginning of the next one. Each rule 5
transformation prints a note, which is also written in the generated code:

```
Note: symbol S: longest match, "a" | "ab" => "a" & "b"?
```

If you really want the short match, there is no reason to write the long
alternative at all.

Rule 5 applies to alternatives of the same choice. It does not look into
other rules: in `X = "a"; Y = "a" & "b"; S = (X | Y) & "c";`, `Y` is
never tried after `X` matched — write `Y | X`.

## Warnings

Warnings do not stop the compile. Removed alternatives are removed from the
generated code too.

| Warning | Example | Meaning |
|---------|---------|---------|
| `symbol X is not used` | `X = "q";` with no reference and no `@` | the rule is generated but nothing calls it |
| `alternative B can never match, A always matches` | `E = A? \| B;` | `A?` always succeeds, so `B` is unreachable; removed |
| `alternative "c" can never match, "c" matches first` | `E = "c" \| "b" \| "c";` | the same alternative again (not a neighbour); removed |
| `alternative "ab" can never match, "a" matches first` | `E = "a" \| "b" \| "ab";` | a literal that starts with an earlier literal, not a neighbour; removed |
| `alternative A has no effect` | `A = A \| "y";` | a left recursive alternative with nothing after the recursion; removed |
| `... repeats an expression that can match empty input` | `(!"x")*` | the loop stops after the first iteration that consumes nothing |

When you see one of the `can never match` warnings, the grammar usually has
the alternatives in the wrong order: put the longer or more specific one
first.

## Errors

Any error stops the compile with exit code `1` and no files written.

| Error | Cause |
|-------|-------|
| `Syntax error line: N!` | the file is not in the grammar language; `N` is the line of the farthest position read |
| `parser-stack-overflow line: N!` | the grammar file nests groups too deeply (about 250 levels of parentheses) |
| `Invalid statement for symbol X` | malformed statement, for example both `\|` and `&` before `=` |
| `Invalid expression for symbol X` | malformed expression |
| `Symbol X is defined more than once` | two `=` definitions |
| `Symbol X must be defined before using \|=` (or `&=`) | extension before the definition |
| `Symbol Unknown is reserved, TokenType::Unknown is the type of no symbol` | a rule named `Unknown` |
| `Symbol int is a C++ keyword` | a rule named like a C++ keyword |
| `String "a\0b" contains a NUL byte, symbol X` | a literal or interval bound with a `\0` / `\x00` byte |
| `Interval ["x","y"] strings must be not empty and of the same length, symbol X` | bad interval |
| `Invalid count interval {3,2} for symbol X` | `n < m` |
| `Number too big N for symbol X` | count above 1000000 |
| `No symbol is selected for the parser table, use @Symbol` | no `@` rule |
| `Symbol X is not defined, used by Y` | reference to a missing rule |
| `Symbol X is left recursive and has no alternative to start with` | `E = E & ...` only |
| `Left recursion A -> B -> A` | indirect left recursion, or left recursion behind a part that can match nothing |
