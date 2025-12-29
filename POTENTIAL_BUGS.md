# Potential Bugs Report

This document lists potential bugs identified during code review. These should be investigated and fixed as appropriate.

---

## 1. Missing `goto fetch` after `op_eos` (CRITICAL)

**File:** `src/lang/exec.c:254-258`

**Issue:** The `op_eos` label is missing a `goto fetch` statement, causing it to fall through into `op_zero`.

```c
 op_eos:
  stack_push(rls, EOS);
                        // <-- Missing: goto fetch;
 op_zero:
  stack_push(rls, RL_ZERO);
  goto fetch;
```

**Impact:** When the End-of-Stream value is pushed, it will also immediately push a zero value, corrupting the stack.

**Suggested fix:** Add `goto fetch;` after `stack_push(rls, EOS);`

---

## 2. Uninitialized variable use in `op_pjump_t` (CRITICAL)

**File:** `src/lang/exec.c:355-364`

**Issue:** The `op_pjump_t` instruction reads `x` without first setting it, using a stale value from a previous instruction.

```c
 op_pjump_t:
  argx = next_op(rls);

  if ( !is_falsey(x) )    // <-- 'x' is NOT set here, unlike op_pjump_f
    rls->pc += argx;

  else
    stack_pop(rls);

  goto fetch;
```

Compare to the correct `op_pjump_f` implementation:
```c
 op_pjump_f:
  argx = next_op(rls);
  x = tos(rls);           // <-- correctly reads top of stack

  if ( is_falsey(x) )
    rls->pc += argx;
  else
    stack_pop(rls);

  goto fetch;
```

**Impact:** The `or` special form will behave incorrectly, as `op_pjump_t` is used to implement short-circuit evaluation for `or`.

**Suggested fix:** Add `x = tos(rls);` before the `if` statement.

---

## 3. Wrong variable in division-by-zero check for `op_rem` (MEDIUM)

**File:** `src/lang/exec.c:517-521`

**Issue:** The modulo operation checks `ry` for division by zero, but the operation uses `ix` and `iy`.

```c
 op_rem:
  ix = as_num_s(rls, ARGS[0]);
  iy = as_num_s(rls, ARGS[1]);
  require(rls, ry != 0, "division by zero");   // <-- should be 'iy', not 'ry'
  stack_push(rls, tag_num(ix % iy));
  goto op_return;
```

**Impact:** The division-by-zero check uses the wrong variable (`ry` instead of `iy`), which may contain a stale value from a previous floating-point operation. This could allow actual division by zero or falsely trigger an error.

**Suggested fix:** Change `ry` to `iy`.

---

## 4. Extra argument to `require` in `op_head` (LOW)

**File:** `src/lang/exec.c:594`

**Issue:** The `require` call has an extra argument "head" that doesn't match the format string.

```c
 op_head:
  x = ARGS[0];
  lx = as_list_s(rls, x);
  require(rls, lx->count > 0, "head", "can't call head on empty list.");
```

**Impact:** Depending on how `require` handles variadic arguments, this could cause undefined behavior or incorrect error messages.

**Suggested fix:** Remove the extra "head" argument:
```c
  require(rls, lx->count > 0, "can't call head on empty list.");
```

---

## 5. Invalid second argument to `require` in `op_tail` (LOW)

**File:** `src/lang/exec.c:602`

**Issue:** The `require` call passes `0` as the second argument instead of part of a format string.

```c
 op_tail:
  x = ARGS[0];
  lx = as_list_s(rls, x);
  require(rls, lx->count > 0, 0, "can't call tail on empty list.");
```

**Impact:** If `require` interprets the second variadic argument as a format string, passing `0` (NULL) could cause a crash or undefined behavior.

**Suggested fix:** Remove the `0`:
```c
  require(rls, lx->count > 0, "can't call tail on empty list.");
```

---

## 6. Closure instruction bytes written to wrong chunk (MEDIUM)

**File:** `src/lang/compile.c:677`

**Issue:** The `OP_CLOSURE` instruction's capture arguments are written to `lchunk->code` (the local/child chunk) instead of the parent `chunk`.

```c
void compile_closure(RlState* rls, List* form, Chunk* chunk, Fun* fun, int* flags, int* line) {
  // ...
  Chunk* lchunk; Method* m;
  // ...
  lchunk = mk_chunk_s(rls, chunk->vars, fun->name, chunk->file);
  // ...
  if ( upvc > 0 ) {
    emit_instr(rls, chunk, OP_CLOSURE, upvc);
    // ...
    // write arguments to closure at once
    code_buf_write(rls, &lchunk->code, buffer, upvc*2);  // <-- should be &chunk->code
  }
```

**Impact:** The closure capture arguments will be written to the child chunk (which is the closure's body) instead of the parent chunk (which executes the `OP_CLOSURE` instruction). This would cause the VM to read garbage as capture arguments.

**Suggested fix:** Change `&lchunk->code` to `&chunk->code`.

---

## Summary

| Severity | Count | Issues |
|----------|-------|--------|
| CRITICAL | 2     | #1, #2 |
| MEDIUM   | 2     | #3, #6 |
| LOW      | 2     | #4, #5 |

The critical issues (#1 and #2) should be addressed immediately as they will cause incorrect runtime behavior. Issue #6 is also likely to cause serious problems with closures that capture variables.
