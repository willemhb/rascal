# This file is for experimenting with different syntaxes and shouldn't be expected to work.

# module system
#
# Every `rascal` file is evaluated with its own toplevel environment. This means that by default, bindings created in one file aren't visible in others.
# (In general, only bootstrapping C code and repl code alters the global environment). A small API is provided for making these bindings visible.
#
# Module environments are cached globally by absolute path name in `Rascal.__loaded__`.
#
# Module bindings are always final, so an error will be raised if attempts are made to assign to them.

# Binds the module environment to the name `Module` in the global environment.
# Raises an error if this file has previously provided a module name.
# Raises an error if the name `Module` is already bound.
provide Module

# Adds the bindings in `Math` to the current module environment.
# Raises na error if the name `Math` has not been previously provided.
require Math

# Binds the module environment provided by `Math` to the name `M` in the current module environment.
# Raises an error if `Math` is not bound or isn't an environment bound by a provide form.
require Math as M

# Checks `Rascal.__loaded__` to see if a file named "local-file.rl" has been loaded.
# If not, it loads the file and saves the resulting module environment in `Rascal.__loaded__`.
# The module environment associated with "local-file.rl" then has its bindings merged into the
# current module environment.
# "local-file.rl" is first expanded to an absolute path name.
# Errors are raised for the reasons you might imagine (file not found, eval errors, etc).
require "local-file.rl"

# As above, but the module environment associated with "local-file.rl" is bound to the 
require "local-file.rl" as Local

# annotation syntax
provide Macro

# Symbols prefixed with the `@` character are treated as annotations.
# Annotations are attached to an object's metadata during compilation,
# rather than being evaluated as expressions.
# This provides a simple universal interface for controlling compilation,
# as well as a way to implement decorators in a more sophisticated version of the language.

# By default, annotations are attached to the current environment. This will be the module environment at toplevel.

@doc "Simple macro utilities."

# Inside a function definition, annotations are attached to the metadata of the function environment.
fun syntax_type(expr) do
  @doc "Classify the syntactic type of an AST expression."

  val expr_type = typeof(expr)

  cond do
    expr_type == :List ->
      :Call
    expr_type == :Tuple ->
      :Reference
    otherwise ->
      :Literal
  end
end

val SyntaxTable = MutMap()

# Annotations can also be added to an object explicitly using the `@<object>.<annot> <value>` syntax.
@SyntaxTable.constraint MutMap
@SyntaxTable.doc "Macro environment (just for demo purposes, macros probably won't be implemented this way)."

# Looping syntax
fun partition(xs) do
  @doc "return a tuple of two lists where"
       "the first list has all of the even-indexed elements from `xs`"
       "and the second list has all of the odd-index elements from `xs`."

  label do
    even(xs, left, right) ->
      if empty?(xs) do
        {left, right}
      else
        odd(rest(xs), conj(left, first(xs)), right)
      end

    odd(xs, left, right) ->
      if empty?(xs) do
        {left, right}
      else
        even(rest(xs), left, conj(right, first(xs)))
      end

    begin xs = seq(xs), left = [], right = [] ->
      even(xs, left, right)
  end
end

# Effect syntax
fun safe_divide(x, y) do
  control _ do
    x / y
  handle
    error.zero_division() ->
      print("error: division by zero in #{x} / #{y}")
      nan
  end
end

fun with_output_file(path, func) do
  control with in do
  when
    enter ->
      in = open(path)
    leave ->
      close(in)
      in = nul
    return(x) ->
      close(in)
      x
  handle _
    error.file_not_found(msg) ->
      print("file at '#{path}' does not exist.")
      print(msg)
      nul
    error.*(msg) ->
      print(msg)
      nul
  begin
    func(in)
  end
end

# Examples to demonstrate using effects to implement exceptions
mac throw(expression) do
  quote do: raise $(Macro.qualify(:error, expression))
end

mac try(do: do_block, catch: catch_block) do
  fun xform(left, right) do
    quote do
      $(Macro.qualify(:error, left)) ->
        $(right)
    end
  end

  val xformed_catch = Macro.arrow_map(xform, catch_block)

  quote do
    control do
      $(do_block)
    handle _
      $(xformed_catch)
    end
  end
end

mac try(do: do_block, catch: catch_block, finally: finally_block) do
  fun xform(left, right) do
    quote do
      $(qualify(:error, left)) ->
        $(right)
    end
  end

  val xformed_catch = Macro.arrow_map(xform, catch_block)

  quote do
    control do
    when
      leave ->
        $(finally_block)
    handle _
      $(xformed_catch)
    begin
      $(do_block)
    end
  end
end

# Examples to demonstrate how effects could be used to implement
# Python-style generators. This isn't really idiomatic in Rascal,
# but it's a good way to experiment with and demonstrate syntax.

# NB: some of the functions below use a spread syntax and variadic arguments,
# but I'm not sure this will actually be part of an initial release of
# Rascal, since it makes multimethods significantly more complicated.
# It should be possible to write a function or macro that generates an argument
# signature for a wrapper function but I don't want to think about this just yet.
mac yield(expression) do
  quote do: raise generator.yield($(expression))
end

fun make_generator(func: Func) do
  fun wrapper(args..) do
    var resume = nul
    var result = nul
    var done?  = false

    fun inner(args..) do
      cond do
        done? ->
          result
        resume /= nul ->
          resume(args..)
        otherwise ->
          control do
          when
            return(x) ->
              result = x
              done?  = true
              resume = nul
          handle k
            generator.yield(x) ->
              resume = k
              x
          begin
            func(args..)
          end
      end
    end
  end

  wrapper
end

# Example to demonstrate how effects could be used to implement coroutines
type DequeNode = (
  mut data: Any,
  mut next: DequeNode | nul
)

type Deque = (
  mut first: DequeNode | nul,
  mut last:  DequeNode | nul
)

type Promise = (
  expression: Func,
  awaiting: Ctl,
  done?: Bool,
  result: Any
)

fun append(d: Deque, x) do
  val node = DequeNode(x, nul)

  if d.first == nul do # empty deque
    d.first = node
    d.last  = node
  else
    d.last.next = node
    d.last      = node
  end
end

fun pop(d: Deque) do
  if d.first == nul do
    throw empty_collection("Empty deque.")
  else
    val out = d.first.data
    d.first = d.first.next

    if d.first == nul do # Deque had just one item
      d.last = nul
    end
  end
end

val CoroutineQueue = Deque()

mac await(expression) do
  quote do: raise coroutine.await( () -> $(expression) )
end

mac async(fun: name, args, do: body) do
  @doc "Example of python style coroutine."

  quote do
    fun $(name) $(args) do
      control do
      handle k
        coroutine.await(expression) ->
          schedule(k, expression)
          nul
      end
    end
  end
end
