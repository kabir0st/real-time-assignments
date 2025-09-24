import math

x = 5
n = 100
result = 0

factorials = {}


def calculate_factorial(num):
    if num in factorials:
        return factorials[num]
    if num in [0, 1]:
        return 1
    factorials[num] = num * calculate_factorial(num - 1)
    return factorials[num]


for i in range(n):
    result += (x**i) / calculate_factorial(i)

actual = math.exp(2)  # the actual solution computed using math library
# Taylor series expansion for e^x at x=2
print(f"Taylor {actual}")
