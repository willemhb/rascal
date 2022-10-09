#!/usr/bin/python3.10

from functools import singledispatch

FNV64_OFFSET = 0xcbf29ce484222325
FNV64_PRIME = 0xcbf29ce484222325


@singledispatch
def fnvhash(_):
    raise NotImplementedError


@fnvhash.register
def _(s: str) -> int:
    return fnvhash(s.encode())


@fnvhash.register
def _(b: bytes) -> int:
    curr: int = 0
    cap:  int = len(b)
    l: list = []

    while cap-curr >= 4:
        chunk: int = b[curr] | (b[curr+1] << 8) | (b[curr+2] << 16) | (b[curr+3] << 24)
        l.append(chunk)
        curr += 4

    if cap-curr >= 2:
        chunk = b[curr] | (b[curr+1] << 8)
        l.append(chunk)
        curr += 2

    if cap-curr == 1:
        l.append(b[curr])

    return fnvhash(l)


@fnvhash.register
def _(ls: list) -> int:
    hashed: int = FNV64_OFFSET

    for i in ls:
        hashed ^= i
        hashed *= FNV64_PRIME

    return hashed
