# Ghoul

Ghoul is a scripting language designed to be terse, simple, expressive and fast. Crafted for personal use, it's not intended to be used in production environments.

## Example of Ghoul

```
:say_hello(to) {
  print "Hello, " ++ to ++ "!";
}

:name = "Ghoul";
say_hello(name);
```

## Learn Ghoul

Check out [learn.md](docs/learn.md)!

## Build

Build with `make`.

Tested on Ubuntu and Windows (MSYS2 UCRT64 | CLANG64)

### Dependencies

* [GNU Readline](https://en.wikipedia.org/wiki/GNU_Readline)
* [libcurl](https://curl.se/libcurl/)

## Test

Test with `make test`.

### Test Dependencies

* [Go](https://go.dev/)

## Thanks

Ghoul's implementation is based on CLox from [Crafting Interpreters](https://craftinginterpreters.com/).
