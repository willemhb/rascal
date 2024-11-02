(module 'shape
  "Types and interfaces for working with shapes in varying dimensions."
  (import 'core)
  (import 'math :as m)
  (export '[Shape Shape2D Rectangle Circle Triangle])
  (export '[area perimeter shape? shape-2d? rectangle? circle? triangle?])

  (type Shape
    "Base of abstract type hierarchy for shape types.")

  (type (Shape Shape2D)
    "Represents shapes in two dimensions.")

  (val (Real root-3)
    "The square root of 3."
    (m/sqrt 3))

  (fun area
    "Calculate the area of a Shape object."
    [(Shape2D s)])

  (fun perimeter
    "Calculate the perimeter of a Shape object."
    [(Shape2D s)])

  (fun shape?
    "Type predicate for Shape."
    [x] (isa? Shape x))

  (fun shape-2d?
    "Type predicate for Shape2D."
    [x] (isa? Shape2D x))

  (type (Shape2D Rectangle)
    "Represents a rectangle."
    [length, height])

  (fun rectangle?
    "Type predicate for Rectangles."
    [x] (isa? Rectangle x))

  (type (Shape2D Circle)
    "Represents a circle."
    [radius])

  (type (Shape2D Triangle)
    "Represents an equilateral triangle."
    [length])

  (fun height
    "Calculate the height an equilateral triangle."
    [(Triangle t)]
    (* t.length (m/hlv root-3)))

  (method area
    "Specialize for Rectangle."
    [(Rectangle r)]
    (* r.length r.height))

  (method area
    "Specialize for Circle."
    [(Circle c)]
    (* pi (sqr c.radius)))

  (method area
    "Specialize for Triangle."
    [(Triangle t)]
    (m/hlv (* (height t) (m/hlv t.length)))))