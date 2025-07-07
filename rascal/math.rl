# Math standard library module (such as it is).

provide Math

val pi = 3.14159
val e  = 2.71828

fun even?(n) do
  n % 2 == 0
end

fun odd?(n) do
  n % 2 == 1
end

fun divides?(a, b) do
  b % a == 0
end

fun abs(x) do
  if x < 0 do
    -x
  else
    x
  end
end

fun max(x, y)
  if x > y do
    x
  else
    y
  end
end

fun min(x, y)
  if x < y do
    x
  else
    y
  end
end

fun pow(x, n) do
  # fast power algorithm (log2)
  fun loop(x, n, acc) do
    if n == 0 do
      acc
    elif even?(n)
      loop(x*x, n/2, acc)
    else
      loop(x, n-1, x*acc)
    end
  end

  loop(x, n, 1)
end

fun gcd(a, b) do
  # euclid's algorithm
  fun loop(a, b) do
    if divides?(a, b) do
      a
    else
      loop(b % a, a)
    end
  end

  val x = abs(a)
  val y = abs(b)

  if x > y do
    loop(y, x)
  else
    loop(x, y)
  end
end
