* <strike>Refactor so that function head is evaluated last (at TOS when ready to call)</strike>
* Finish refactoring code that references stack
* Make sure assignments are handled correctly when compiling sequences (avoid extra pop())
* Make sure assignments are disallowed where appropriate:
    * <strike>top level of `if` forms</strike>
    * catch/throw expressions 
    * function arguments
