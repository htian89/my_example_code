#!/usr/bin/env python3

print("hello")

x = ['zebra', 49, -87, "aardddd", 200]
x.append("more")
print(x)
list.append(x, "extra")
print(x)

def fun():
    print("fun")

fun()

five = 5
two = 2
zero = 0
boolx = (five and two)
if boolx:
    print(boolx)
else:
    pass
