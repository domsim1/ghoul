# Math Library Documentation

The Math library provides mathematical functions and constants for the Ghoul programming language. Load it using `use "Math";`.

## Constants

### Mathematical Constants
- `Math.PI` - Pi (π ≈ 3.14159265358979)
- `Math.E` - Euler's number (e ≈ 2.71828182845905)
- `Math.SQRT2` - Square root of 2 (√2 ≈ 1.41421356237310)

## Number Classification Functions

### Math.isFinite(value)
Tests if a number is finite (not infinite or NaN).
```ghoul
use "Math";

print Math.isFinite(42.5);     # true
print Math.isFinite(1.0/0.0);  # false (infinity)
print Math.isFinite(0.0/0.0);  # false (NaN)
```

### Math.isNaN(value)
Tests if a value is NaN (Not a Number).
```ghoul
use "Math";

print Math.isNaN(0.0/0.0);     # true
print Math.isNaN(42.5);        # false
```

### Math.isInfinite(value)
Tests if a number is infinite (positive or negative).
```ghoul
use "Math";

print Math.isInfinite(1.0/0.0);   # true
print Math.isInfinite(-1.0/0.0);  # true
print Math.isInfinite(42.5);      # false
```

### Math.sign(value)
Returns the sign of a number (-1, 0, or 1).
```ghoul
use "Math";

print Math.sign(42.5);     # 1
print Math.sign(-3.7);     # -1
print Math.sign(0.0);      # 0
```

## Advanced Rounding Functions

### Math.fmod(x, y)
Returns the floating-point remainder of x/y (with same sign as x).
```ghoul
use "Math";

print Math.fmod(7.5, 3.0);    # 1.5
print Math.fmod(-7.5, 3.0);   # -1.5
```

### Math.remainder(x, y)
Returns the IEEE remainder of x/y (rounds to nearest integer).
```ghoul
use "Math";

print Math.remainder(7, 3);   # 1
print Math.remainder(8, 3);   # -1 (rounds to nearest)
```

### Math.modf(value)
Returns the integer and fractional parts as a list [integer, fractional].
```ghoul
use "Math";

print Math.modf(3.14159);   # [3, 0.14159]
print Math.modf(-1.25);     # [-1, -0.25]
```

## Statistical Functions

### Math.clamp(value, min, max)
Constrains a value between minimum and maximum bounds.
```ghoul
use "Math";

print Math.clamp(10, 0, 5);    # 5 (clamped to max)
print Math.clamp(-5, 0, 5);    # 0 (clamped to min)
print Math.clamp(3, 0, 5);     # 3 (within bounds)
```

### Math.lerp(start, end, t)
Linear interpolation between start and end values.
```ghoul
use "Math";

print Math.lerp(0, 10, 0.5);   # 5 (halfway)
print Math.lerp(10, 20, 0.3);  # 13 (30% of the way)
print Math.lerp(10, 20, 2);    # 30 (extrapolation)
```

### Math.map(value, in_min, in_max, out_min, out_max)
Maps a value from one range to another.
```ghoul
use "Math";

print Math.map(5, 0, 10, 0, 100);      # 50
print Math.map(0.3, 0, 1, 10, 20);     # 13
```

## Angle Conversion Functions

### Math.degrees(radians)
Converts radians to degrees.
```ghoul
use "Math";

print Math.degrees(Math.PI);       # 179.999847960504
print Math.degrees(Math.PI / 2);   # 89.9999239802521
```

### Math.radians(degrees)
Converts degrees to radians.
```ghoul
use "Math";

print Math.radians(180);   # 3.14159... (π)
print Math.radians(90);    # 1.5708... (π/2)
```

## Random Number Functions

### Math.seed(value)
Seeds the random number generator for reproducible sequences.
```ghoul
use "Math";

Math.seed(12345);
```

### Math.random()
Returns a random floating-point number between 0.0 and 1.0.
```ghoul
use "Math";

:rand_val = Math.random();   # e.g., 0.7283...
```

### Math.randomInt(min, max)
Returns a random integer between min and max (inclusive).
```ghoul
use "Math";

print Math.randomInt(1, 10);     # Random integer from 1 to 10
print Math.randomInt(-5, 5);     # Random integer from -5 to 5
```

### Math.randomRange(min, max)
Returns a random floating-point number between min and max.
```ghoul
use "Math";

print Math.randomRange(1.5, 10.5);   # Random float between 1.5 and 10.5
```

## Bitwise Math Functions

### Math.clz32(value)
Counts leading zero bits in 32-bit binary representation.
```ghoul
use "Math";

print Math.clz32(1);          # 31 (binary: 00000000000000000000000000000001)
print Math.clz32(8);          # 28 (binary: 00000000000000000000000000001000)
```

### Math.imul(a, b)
32-bit integer multiplication (handles overflow correctly).
```ghoul
use "Math";

print Math.imul(3, 5);        # 15
print Math.imul(-3, 5);       # -15
```

## Logarithm Functions

### Math.logb(value, base)
Logarithm with arbitrary base.
```ghoul
use "Math";

print Math.logb(8, 2);        # 3 (log₂(8) = 3)
print Math.logb(100, 10);     # 2 (log₁₀(100) = 2)
print Math.logb(81, 3);       # 4 (log₃(81) = 4)
```

## Special Functions

### Math.factorial(n)
Calculates the factorial of a non-negative integer.
```ghoul
use "Math";

print Math.factorial(0);      # 1
print Math.factorial(5);      # 120
print Math.factorial(3);      # 6
```

### Math.gamma(x)
Gamma function (Γ(n) = (n-1)! for positive integers).
```ghoul
use "Math";

print Math.gamma(1);          # 1 (Γ(1) = 0!)
print Math.gamma(4);          # 6 (Γ(4) = 3!)
```

### Math.gcd(a, b)
Greatest common divisor of two integers.
```ghoul
use "Math";

print Math.gcd(18, 24);       # 6
print Math.gcd(14, 8);        # 2
print Math.gcd(-6, 9);        # 3 (handles negatives)
```

### Math.lcm(a, b)
Least common multiple of two integers.
```ghoul
use "Math";

print Math.lcm(3, 4);         # 12
print Math.lcm(6, 8);         # 24
```

## Core Math Functions (inherited from base)

The Math library also includes all standard mathematical functions:

### Basic Operations
- `Math.abs(x)` - Absolute value
- `Math.min(a, b)` - Minimum of two values
- `Math.max(a, b)` - Maximum of two values

### Trigonometric Functions
- `Math.sin(x)` - Sine
- `Math.cos(x)` - Cosine  
- `Math.tan(x)` - Tangent

### Exponential and Logarithmic
- `Math.exp(x)` - e^x
- `Math.log(x)` - Natural logarithm
- `Math.log10(x)` - Base-10 logarithm
- `Math.pow(x, y)` - x raised to power y
- `Math.sqrt(x)` - Square root

### Rounding Functions
- `Math.floor(x)` - Round down
- `Math.ceil(x)` - Round up
- `Math.round(x)` - Round to nearest integer

## Usage Examples

### Statistical Analysis
```ghoul
use "Math";

:data = [1.2, 3.4, 2.1, 4.5, 2.8];
:min_val = Math.min(Math.min(Math.min(Math.min(data[0], data[1]), data[2]), data[3]), data[4]);
:max_val = Math.max(Math.max(Math.max(Math.max(data[0], data[1]), data[2]), data[3]), data[4]);

# Normalize data to 0-1 range
for (:i = 0; i < 5; i += 1) {
    :normalized = Math.map(data[i], min_val, max_val, 0, 1);
    print "Value " ++ String(data[i]) ++ " normalized: " ++ String(normalized);
}
```

### Random Number Generation
```ghoul
use "Math";

# Seed for reproducible results
Math.seed(42);

# Generate random game coordinates
for (:i = 0; i < 10; i += 1) {
    :x = Math.randomInt(0, 800);
    :y = Math.randomInt(0, 600);
    :speed = Math.randomRange(1.0, 5.0);
    
    print "Entity " ++ String(i) ++ ": pos(" ++ String(x) ++ ", " ++ String(y) ++ "), speed: " ++ String(speed);
}
```

### Mathematical Calculations
```ghoul
use "Math";

# Convert between coordinate systems
:angle_degrees = 45;
:angle_radians = Math.radians(angle_degrees);
:x = Math.cos(angle_radians) * 10;
:y = Math.sin(angle_radians) * 10;

print "45° point on circle: (" ++ String(x) ++ ", " ++ String(y) ++ ")";

# Calculate factorial and gamma relationship
for (:n = 1; n <= 5; n += 1) {
    :fact = Math.factorial(n);
    :gamma = Math.gamma(n + 1);
    print "n=" ++ String(n) ++ ": " ++ String(n) ++ "! = " ++ String(fact) ++ ", Γ(" ++ String(n+1) ++ ") = " ++ String(gamma);
}
```