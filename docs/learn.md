# Learn to Code in Ghoulish

To get started, lets make a simple "Hello, Ghoul!" program.

Create a new file called `hello.ghoul`, ghoul scripts should use the file extention `.ghoul`.

Now lets add the following to our `hello.ghoul` file.

```
print "Hello, Ghoul!";
```

The script can be ran fromt he command line with `ghoul ./hello.ghoul`.


Congratulation, you just coded your first words in ghoulish!

## Types

Ghoul has many.

* `num`: Double precision floating point
* `bool`: true or false: `true`
* `nil`: null: `nil`
* `inst`: An instance of an object: `Pizza()`
* `str`: Collection of characters: `"hello"`
* `class`: Blueprint for creating an inst: `:Pizza{}`
* `list`: dynmaic array: `[1, 2, 3]`
* `fn`: function: `:do_thing() {}`

Types can be checked in using the `is<type>` functions.


Example:

```
print isnum(10);
# true
```


An `inst` can be check using the `instof` function.


Example:

```
:Pizza { init() {} }
:pizza = Pizza();
print instof(pizza, Pizza);
# true
```


Exluding `num`, `nil`, `bool`, `class`, `fn` and `inst` all types are also an `inst`. This allows methods to live on those types.

```
print "apple".len();
# 5
```


Ghoul also has some additional built in classes that provide some special functionality.

* `File`: for file IO
* `Error`: keeps a stacktrace
* `Map`: Simple Key Value store: `:phone_book = { "dom": "04-XXX-XXX-XXX" }`


## The Summon Operator

All new creations in Ghoul start with the `:` (summon) operator. The `:` operator is how we define variables, functions, classes and lambdas. Global identifies in Ghoul must be unique.


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

Summon functions like so `:<identifier>(<?parameters,>) {}`. To return a value from a function use `->`. All functions in Ghoul are [closures](https://en.wikipedia.org/wiki/Closure_(computer_programming)), hence are [fist-class](https://en.wikipedia.org/wiki/First-class_function).

Example:

```
:add(a, b) {
	-> a + b;
}

print add(10, 20);
# 30
```


### Class

Summoning classes can be done like so `:<identifier> {}`. Classes can inherit from each other using `<`. Like functions classes are first-class so can be treated like any other value. The constructor for a class can be set by defining creating an `init(<?parameters,>)` method. Methods and properties on a class do not need to be summoned. Methods in classes can refrence the class instance by using the `this`. An inherited class can be accessed using `super`.


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


## WIP