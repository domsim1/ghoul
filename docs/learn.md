# Learn to Code in Ghoulish

To get started, let's make a simple "Hello, Ghoul!" program.

Create a new file called `hello.ghoul`. Ghoul scripts should use the file extension `.ghoul`.

Now let's add the following to our `hello.ghoul` file:

```ghoul
print "Hello, Ghoul!";
```

The script can be run from the command line with `ghoul ./hello.ghoul`.

Congratulations, you just coded your first words in Ghoulish!



## Types

Ghoul has many.

* `num`: Double precision floating point
* `bool`: true or false: `true`
* `nil`: null: `nil`
* `inst`: An instance of an object: `Pizza()`
* `str`: Collection of characters: `"hello"`
* `class`: Blueprint for creating an inst: `:Pizza{}`
* `list`: dynamic array: `[1, 2, 3]`
* `fn`: function: `:do_thing() {}`
* `Map`: Simple Key Value store: `:phone_book = { "dom": "04-XXX-XXX-XXX" }`

Types can be checked using the `is<type>` functions.


Example:

```
print isnum(10);
# true
```


An `inst` can be checked using the `instof` function.


Example:

```
:Pizza { init() {} }
:pizza = Pizza();
print instof(pizza, Pizza);
# true
```


Excluding `num`, `nil`, `bool`, `class` and `fn`, all types are also an `inst`. This allows methods to live on those types.

```
print "apple".len();
# 5
```


Ghoul also has some additional built in classes that provide some special functionality.

* `File`: for file IO
* `Error`: works with `panic()`


## The Summon Operator

All new creations in Ghoul start with the `:` (summon) operator. The `:` operator is how we define variables, functions, classes and lambdas. Global identifiers in Ghoul must be unique.


### Variable

The syntax to summon a variable follows `:<identifier>;`, this will create a variable with the value `nil`. Variables can also be created with an assignment like so `:<identifier> = 10;`, this will create a variable with the value `10`.

Example:

```
:a = 10;
:b = 20;
print a + b;
# 30
```


### Function

Summon functions like so `:<identifier>(<?parameters,>) {}`. To return a value from a function use `->`. All functions in Ghoul are [closures](https://en.wikipedia.org/wiki/Closure_(computer_programming)), hence are [first-class](https://en.wikipedia.org/wiki/First-class_function).

Example:

```
:add(a, b) {
  -> a + b;
}

print add(10, 20);
# 30
```

### Class

Summoning classes can be done like so `:<identifier> {}`. Classes can inherit from each other using `<`. Like functions classes are first-class so can be treated like any other value. The constructor for a class can be set by defining an `init(<?parameters,>)` method. Methods and properties on a class do not need to be summoned. Methods in classes can reference the class instance by using `this`. An inherited class can be accessed using `super`.


Example:

```
:Animal {
  init(name) {
    this.name = name;
    this.legs = 0;
  }
  leg_count() {
    -> this.legs;
  }
}

:Dog < Animal {
  init() {
    super.init("Dog");
    this.legs = 4;
  }
}

:dog = Dog();

print dog.leg_count();
# 4
print dog.name;
# Dog
```


### Lambda

Lambdas are functions without identifiers `:(<?parameters,>) {}`.


Example:

```
:make_adder(to_add) {
  -> :(x) { -> to_add + x; }
}
:add10 = make_adder(10);
print add10(20);
# 30
```

## Comparisons

The following comparisons are supported.

* Equal: `==`
* Greater: `>`
* Less: `<`
* Greater or Equal: `>=`
* Less or Equal: `<=`
* Not Equal: `!=`

Ghoul also supports `!` for not.


## If

Example:

```
:hp = 15;

if (hp < 10) {
  print "low HP!";
} else if (hp > 20) {
  print "high HP!";
} else {
  print "normal HP!";
}
# normal HP!
```


## Loops

Ghoul has three types of loops `while`, `for` and `generic for`. Loops also support `continue` and `break` keywords for flow control.


### While

Loops until condition is false `while (<condition>) {}`.


Example:

```
while (true) {
  print "hi";
}
# will print hi forever
```


### For

Good old C style for loop.


Example:

```
for (:i = 0; i < 10; i += 1) {
  print i;
}
# will print from 0 to 9
```


### Generic For

Loops over a `list`, `string` or `generator function`.


Example:

```
for (:num in [1, 2, 3, 4]) {
  print num;
}
# will print 1 to 4
```

## Map

Simple key value pair.

Example:

```
:phone_book = {
  "Stacy's mum": "XX-XXX-XXX-XXX",
}

print phone_book["Stacy's mum"];
```

Maps can also be looped over using `keys`, `values` or `pairs` generator functions.

Example:

```
:phone_book = {
  "Stacy's mum": "XX-XXX-XXX-XXX",
}

for (:pair in phone_book.pairs()) {
  print "I think I am in love with " ++ pair.key ++ " at " ++ pair.value;
}
```

## Varadic Functions

The last parameter can be marked with a `*` to indicate it's variadic. All additional provided parameters will be packed into a list that can be accessed by the parameter marked with `*`.

Example:

```
:add(*nums) {
  :sum = 0;
  for (:n in nums) {
    sum += n;
  }
  ->sum;
}

print add(1, 2, 3, 4);
# 10
```

## Generator Functions

With the power of closures comes the ability to create generator functions. Generator functions are useful for creating lazy evaluation iterators that can be used with generic for loops.

Example:

```
:fibonacci_generator() {
  :last_number = 0;
  :current_number = 1;
  ->:() {
    :next_number = last_number + current_number;
    last_number = current_number;
    current_number = next_number;
    ->next_number;
  }
}
for (:n in fibonacci_generator()) {
  print n;
  if (n == 21) {
    break;
  }
}
# 1 2 3 5 13 21
```

## Use

In Ghoul you can break up code into files. To consume a file within your script, the keyword `use` is used. The file path can be absolute or relative. In addition, internal libraries such as `Math`, `Request`, or `RL` (Raylib) can be loaded using `use`. Note that a consumed file is loaded into global scope - a file will fail to load if you have declaration collisions.

Example:

```
# file: coolmath.ghoul
:CoolMath {
  PI = 3.14159265359;
}
CoolMath = CoolMath();
```

```
# file: main.ghoul
use "Math";
use "./coolmath.ghoul";
print CoolMath.PI == Math.PI;
# true
```

## Error

Error is a class that can be used to create an error `inst`. Errors play well with `iserr` and `panic` functions for control flow.

```
:err = Error("oh no!");
if (iserr(err)) {
  panic(err);
}
# Error: oh no!
# [line 3 of ghoul/file.ghoul] in script
```


## Operators

Ghoul supports various operators for different operations:

### Arithmetic Operators
* `+` : Addition
* `-` : Subtraction
* `*` : Multiplication
* `/` : Division
* `%` : Modulo

### Assignment Operators
* `=` : Assignment
* `+=` : Add and assign
* `-=` : Subtract and assign
* `*=` : Multiply and assign
* `/=` : Divide and assign
* `%=` : Modulo and assign

### Bitwise Operators
* `&` : Bitwise AND
* `|` : Bitwise OR
* `^` : Bitwise XOR
* `&=` : Bitwise AND and assign
* `|=` : Bitwise OR and assign

### String Operations
* `++` : String concatenation

**Important**: String concatenation (`++`) only works between strings and strings, or lists and lists. To concatenate numbers with strings, convert numbers to strings first using `String()`.

Example:
```ghoul
:name = "Ghoul";
:version = 1.0;
:message = "Welcome to " ++ name ++ " v" ++ String(version);
print message;
# Welcome to Ghoul v1.0

# This would cause an error:
# :bad_message = "Version " ++ version;  # ERROR: Can only concat two strings

# Correct way:
:good_message = "Version " ++ String(version);
print good_message;
# Version 1.0
```

### Type Conversion
Use the built-in type constructors to convert between types:

```ghoul
:num = 42;
:str_from_num = String(num);    # "42"
print "The answer is " ++ str_from_num;

:str = "3.14";
:num_from_str = str.asnum();    # 3.14
print num_from_str * 2;         # 6.28
```

## Native Libraries

Ghoul provides several built-in libraries that can be loaded with `use`:

### Math Library
Provides mathematical functions and constants:
```ghoul
use "Math";
print Math.PI;
print Math.sin(Math.PI / 2);
```

### Request Library
For HTTP requests:
```ghoul
use "Request";
:headers = ["User-Agent: Ghoul-App/1.0"];
:response = Request.get("https://api.example.com/data", headers);
print response.response;  # Response body
print response.status;    # HTTP status code
```

### JSON Library
For JSON parsing and generation:
```ghoul
use "JSON";
:data = { "name": "Ghoul", "version": 1.0 };
:json_string = JSON.stringify(data, false);
:parsed = JSON.parse(json_string);
```

### Raylib Library (RL)
For graphics, input, and game development:
```ghoul
use "RL";
RL.init_window(800, 600, "My Game");
while (!RL.window_should_close()) {
    RL.begin_drawing();
    RL.clear_background(RL.RAYWHITE);
    RL.draw_text("Hello World!", 200, 200, 20, RL.DARKGRAY);
    RL.end_drawing();
}
RL.close_window();
```

## File I/O

Ghoul provides the `File` class for file operations:

```ghoul
# Reading a file
:file = File("data.txt");
:content = file.read();
print content;

# Writing to a file
:output = File("output.txt");
output.write("Hello from Ghoul!");
```

## List Methods

Lists in Ghoul have several useful methods:

```ghoul
:numbers = [1, 2, 3, 4, 5];

# Get length
print numbers.len(); # 5

# Add elements
numbers.push(6);
print numbers; # [1, 2, 3, 4, 5, 6]

# Remove elements
:removed = numbers.pop();
print removed; # 6
print numbers; # [1, 2, 3, 4, 5]

# Access elements
print numbers[0]; # 1
print numbers[-1]; # 5 (last element)
```

## String Methods

Strings have various methods for manipulation:

```ghoul
:text = "Hello, World!";

print text.len(); # 13
print text.upper(); # HELLO, WORLD!
print text.lower(); # hello, world!
print text.slice(0, 5); # Hello
```

## Advanced Examples

### Building a Simple Calculator
```ghoul
:Calculator {
    add(a, b) {
        -> a + b;
    }

    subtract(a, b) {
        -> a - b;
    }

    multiply(a, b) {
        -> a * b;
    }

    divide(a, b) {
        if (b == 0) {
            -> Error("Division by zero");
        }
        -> a / b;
    }
}

:calc = Calculator();
print calc.add(10, 5); # 15
print calc.divide(10, 2); # 5
```

### Working with JSON Data
```ghoul
use "JSON";

:person = {
    "name": "Alice",
    "age": 30,
    "hobbies": ["reading", "coding", "gaming"]
};

:json_str = JSON.stringify(person, false);
print json_str;

:parsed_person = JSON.parse(json_str);
print parsed_person["name"]; # Alice
```

### Simple Game Loop with Raylib
```ghoul
use "RL";

RL.init_window(400, 300, "Simple Game");
RL.set_target_fps(60);

:player_pos = RL.Vector2(200, 150);
:speed = 200;

while (!RL.window_should_close()) {
    :dt = RL.get_frame_time();

    # Input handling
    if (RL.is_key_down(RL.KEY_RIGHT)) {
        player_pos.x += speed * dt;
    }
    if (RL.is_key_down(RL.KEY_LEFT)) {
        player_pos.x -= speed * dt;
    }

    # Drawing
    RL.begin_drawing();
    RL.clear_background(RL.RAYWHITE);
    RL.draw_circle_v(player_pos, 20, RL.RED);
    RL.draw_text("Use arrow keys to move", 10, 10, 20, RL.DARKGRAY);
    RL.end_drawing();
}

RL.close_window();
```

## Best Practices

1. **Use meaningful variable names**: `:player_health` instead of `:ph`
2. **Handle errors gracefully**: Always check for potential errors using `iserr()`
3. **Keep functions small**: Break complex logic into smaller, reusable functions
4. **Use classes for related functionality**: Group related methods and data together
5. **Comment your code**: Use `#` for single-line comments
6. **Organize code with files**: Use `use` to split large programs into modules

## Common Patterns

### Error Handling
```ghoul
:divide_safe(a, b) {
    if (b == 0) {
        -> Error("Cannot divide by zero");
    }
    -> a / b;
}

:result = divide_safe(10, 2);
if (iserr(result)) {
    print "Error: " ++ result.message;
} else {
    print "Result: " ++ String(result);
}
```

### Iterator Pattern
```ghoul
:range(start, end) {
    :current = start;
    ->:() {
        if (current >= end) {
            -> nil;
        }
        :value = current;
        current += 1;
        -> value;
    }
}

for (:i in range(1, 6)) {
    print i; # Prints 1, 2, 3, 4, 5
}
```

This completes the Ghoul language tutorial. You now have all the tools needed to start building applications in Ghoul, from simple scripts to interactive games with graphics and user input!
