import random

def get_int(msg, minimum, default):
    while True:
        try:
            line = input(msg)
            if not line and default is not None:
                return default
            integer = int(line)
            if integer < minimum:
                print("%d < minimum(%d)", integer, minimum)
            else:
                return integer
        except ValueError as err:
            print(err)

rows = get_int("rows:", 1, None)
columns = get_int("columns:", 1, None)
minimum = get_int("minimum (or Enter for 0):", -100000, 0)

defualt_maximum = 1000 if minimum < 1000 else 2 * minimum
maximum = get_int("maxmum (or Enter for " + str(defualt_maximum) + "):", minimum, defualt_maximum)

i = 0
while i < rows:
    row = ""
    j = 0
    while j < columns:
        item = random.randint(minimum, maximum)
        s = str(item)
        while len(s) < 10:
            s = " " + s
        row += s
        j += 1
    print(row)
    i += 1
