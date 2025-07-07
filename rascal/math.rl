# Math standard library module (such as it is).

provide Math

val pi = 3.14159
val e  = 2.71828

fun even?(n)
  n % 2 == 0
end

fun odd?(n)
  n % 2 == 1
end

fun divides?(a, b)
  b % a == 0
end

fun abs(x)
  if x < 0
    -x
  else
    x
  end
end

fun max(x, y)
  if x > y
    x
  else
    y
  end
end

fun min(x, y)
  if x < y
    x
  else
    y
  end
end

fun pow(x, n)
  # fast power algorithm (log2)
  fun loop(x, n, acc)
    if n == 0
      acc
    elif even?(n)
      loop(x*x, n/2, acc)
    else
      loop(x, n-1, x*acc)
    end
  end

  loop(x, n, 1)
end

fun gcd(a, b)
  # euclid's algorithm
  fun loop(a, b)
    if divides?(a, b)
      a
    else
      loop(b % a, a)
    end
  end

  val x = abs(a)
  val y = abs(b)

  if x > y
    loop(y, x)
  else
    loop(x, y)
  end
end