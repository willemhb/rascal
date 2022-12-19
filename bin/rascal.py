# notional rascal prototype written in Python
from enum import Enum
from dataclasses import dataclass

class ValueType(Enum):
    REAL
    INT
    CHAR
    BOOL
    NUL
    SYMBOL
    CONS
    FUNCTION
    STRING
    VECTOR
    SET
    DICT
    RECORD
    MODULE
    PRIMITIVE
    NATIVE
    EFFECT
    PACKAGE
    CLOSURE
    TYPE


@dataclass(slots=True)
class Value:
    type: ValueType


@dataclass(slots=True)
class Real(Value):
    value: float

    def __init__(self, value: float):
        self.type  = ValueType.REAL
        self.value = value


@dataclass(slots=True)
class Int(Value):
    value: int

    def __init__(self, value: int):
        self.type = ValueType.INT
        self.value = value


@dataclass(slots=True)
class Nul(Value):
    value: None = None

    


@dataclass(slots=True)
class Bool(Value):
    value: bool


@dataclass(slots=True)
class Symbol(Value):
    name: str
    idno: int
    hash: int

    def __init__(self, name: str, idno: int):
        self.type = ValueType.SYMBOL
        self.idno = idno
        self.name = name
        self.hash = hash((self.type, self.name, self.idno))

    def __hash__(self):
        return self.hash


@dataclass(slots=True)
class Cons(Value):
    car: Value
    cdr: Value


@dataclass(slots=True)
class Vec(Value):
    values: list

    def __init__(self, *args):
        self.type = VECTOR
        self.values = [*args]


@dataclass(slots=True)
class Dict(Value):
    entries: dict[Value, Value]


    
