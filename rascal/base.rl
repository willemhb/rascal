provide Base
require Core


val loaded = m{}

fun isa?(x, t: Type) do
  instance? typeof(x), t
end

fun map(i: Int, xs: List) do
  map (x) -> x[i], xs
end

fun map(k: Sym, xs: List) do
  map (x) -> x[k], xs
end

fun map(fn: Fun, xs: List) do
  label loop xs=xs, ac=[] do
    case xs do
      []    -> reverse(ac)
      [h|t] -> loop xs, fn(h) | ac
    end
  end
end

mac throw(error_type, error_message) do
  val qualified = qualify(error_type, :exception)
  quote do
    raise $(qualified), $(error_message)
  end
end

mac try(do: try_block, catch: catch_block) do
  quote do
    control do
      $$(try_block)
    handle
      $$(map_catch(catch_block))
    end
  end
end

