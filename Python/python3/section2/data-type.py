#!/usr/bin/env python3
import sys

def equal_float(a, b):
    return abs(a - b) <= sys.float_info.epsilon

print(equal_float(1, 5))

s = 14.25.hex()
print(s)
print("{0}{1}".format("argc1",200))
