# String Class Documentation

The `String` class in Ghoul provides string manipulation capabilities. All string methods are pure functions that return new strings rather than modifying the original.

## Table of Contents

- [Constructor](#constructor)
- [Core Methods](#core-methods)
- [Case Conversion](#case-conversion)
- [Search Methods](#search-methods)
- [Manipulation Methods](#manipulation-methods)
- [Utility Methods](#utility-methods)
- [Examples](#examples)

## Constructor

### `String(value)`

Creates a new string from the given value.

**Parameters:**
- `value` - A string or number to convert to string

**Returns:** String instance

**Examples:**
```ghoul
:str1 = String("hello");      # From string literal
:str2 = String(42);           # From number -> "42"
:str3 = String(3.14);         # From float -> "3.14"
```

## Core Methods

### `len()`

Returns the length of the string.

**Returns:** Number - the length of the string

**Example:**
```ghoul
:text = String("hello");
print text.len();  # 5
```

### `contains(substring)`

Checks if the string contains the specified substring.

**Parameters:**
- `substring` - String to search for

**Returns:** Boolean - true if substring is found, false otherwise

**Example:**
```ghoul
:text = String("hello world");
print text.contains("world");  # true
print text.contains("foo");    # false
```

### `split(delimiter)`

Splits the string into a list using the specified delimiter.

**Parameters:**
- `delimiter` - String to use as separator

**Returns:** List of strings

**Example:**
```ghoul
:csv = String("apple,banana,cherry");
:fruits = csv.split(",");
print fruits;  # ['apple', 'banana', 'cherry']
```

### `asnum()`

Converts the string to a number if possible.

**Returns:** Number if conversion successful, nil otherwise

**Example:**
```ghoul
:num_str = String("42");
:value = num_str.asnum();
print value;  # 42

:invalid = String("abc");
print invalid.asnum();  # nil
```

## Case Conversion

### `lower()`

Converts all characters in the string to lowercase.

**Returns:** String - new string with lowercase characters

**Example:**
```ghoul
:text = String("Hello World!");
print text.lower();  # "hello world!"
```

### `upper()`

Converts all characters in the string to uppercase.

**Returns:** String - new string with uppercase characters

**Example:**
```ghoul
:text = String("Hello World!");
print text.upper();  # "HELLO WORLD!"
```

## Search Methods

### `index_of(substring)`

Finds the first occurrence of a substring.

**Parameters:**
- `substring` - String to search for

**Returns:** Number - index of first occurrence, or -1 if not found

**Example:**
```ghoul
:text = String("hello world hello");
print text.index_of("hello");  # 0
print text.index_of("world");  # 6
print text.index_of("foo");    # -1
```

### `last_index_of(substring)`

Finds the last occurrence of a substring.

**Parameters:**
- `substring` - String to search for

**Returns:** Number - index of last occurrence, or -1 if not found

**Example:**
```ghoul
:text = String("hello world hello");
print text.last_index_of("hello");  # 12
print text.last_index_of("world");  # 6
print text.last_index_of("foo");    # -1
```

### `starts_with(prefix)`

Checks if the string starts with the specified prefix.

**Parameters:**
- `prefix` - String to check for at the beginning

**Returns:** Boolean - true if string starts with prefix, false otherwise

**Example:**
```ghoul
:text = String("hello world");
print text.starts_with("hello");  # true
print text.starts_with("world");  # false
print text.starts_with("");       # true (empty string)
```

### `ends_with(suffix)`

Checks if the string ends with the specified suffix.

**Parameters:**
- `suffix` - String to check for at the end

**Returns:** Boolean - true if string ends with suffix, false otherwise

**Example:**
```ghoul
:text = String("hello world");
print text.ends_with("world");  # true
print text.ends_with("hello");  # false
print text.ends_with("");       # true (empty string)
```

## Manipulation Methods

### `trim()`

Removes whitespace from both ends of the string.

**Returns:** String - new string with leading and trailing whitespace removed

**Example:**
```ghoul
:text = String("  hello world  ");
print text.trim();  # "hello world"

:no_spaces = String("hello");
print no_spaces.trim();  # "hello" (returns same string)
```

### `substring(start, end)`

Extracts a portion of the string between start and end indices.

**Parameters:**
- `start` - Number - starting index (inclusive)
- `end` - Number (optional) - ending index (exclusive). If omitted, extracts to end of string

**Returns:** String - extracted substring

**Notes:**
- Negative indices are clamped to 0
- Indices beyond string length are clamped to string length
- If start > end, start is set to end (returns empty string)

**Examples:**
```ghoul
:text = String("hello world");
print text.substring(0, 5);   # "hello"
print text.substring(6);      # "world"
print text.substring(1, 4);   # "ell"
print text.substring(-1, 3);  # "hel" (start clamped to 0)
print text.substring(5, 3);   # "" (start > end)
```

### `replace(search, replacement)`

Replaces the first occurrence of search string with replacement string.

**Parameters:**
- `search` - String to search for
- `replacement` - String to replace with

**Returns:** String - new string with first occurrence replaced

**Example:**
```ghoul
:text = String("hello world hello");
print text.replace("hello", "hi");  # "hi world hello"
print text.replace("foo", "bar");   # "hello world hello" (no change)
```

### `replace_all(search, replacement)`

Replaces all occurrences of search string with replacement string.

**Parameters:**
- `search` - String to search for
- `replacement` - String to replace with

**Returns:** String - new string with all occurrences replaced

**Limits:** Maximum 10,000 replacements to prevent resource exhaustion

**Example:**
```ghoul
:text = String("foo bar foo baz foo");
print text.replace_all("foo", "test");  # "test bar test baz test"
print text.replace_all("xyz", "abc");   # "foo bar foo baz foo" (no change)
```

## Utility Methods

### String Indexing

You can access individual characters using bracket notation:

**Syntax:** `string[index]`

**Returns:** String - single character at the specified index

**Example:**
```ghoul
:text = String("hello");
print text[0];  # "h"
print text[1];  # "e"
print text[4];  # "o"
```

### String Concatenation

Use the `++` operator to concatenate strings:

**Syntax:** `string1 ++ string2`

**Example:**
```ghoul
:greeting = String("Hello");
:name = String("World");
:message = greeting ++ " " ++ name ++ "!";
print message;  # "Hello World!"

# Convert numbers to strings for concatenation
:count = 42;
:result = String("Count: ") ++ String(count);
print result;  # "Count: 42"
```

## Examples

### Text Processing
```ghoul
:text = String("  Hello, World!  ");
:cleaned = text.trim().lower();
print cleaned;  # "hello, world!"

:words = cleaned.split(", ");
print words;  # ['hello', 'world!']
```

### Search and Replace
```ghoul
:template = String("Hello {name}, welcome to {place}!");
:personalized = template.replace("{name}", "Alice").replace("{place}", "Ghoul");
print personalized;  # "Hello Alice, welcome to Ghoul!"
```

### Validation
```ghoul
:email = String("user@example.com");
:is_valid = email.contains("@") && email.ends_with(".com");
print is_valid;  # true

:domain_start = email.index_of("@") + 1;
:domain = email.substring(domain_start);
print domain;  # "example.com"
```

### Text Analysis
```ghoul
:text = String("The quick brown fox jumps over the lazy dog");
:word_count = text.split(" ").len();
print word_count;  # 9

:has_fox = text.contains("fox");
print has_fox;  # true

:uppercase = text.upper();
print uppercase;  # "THE QUICK BROWN FOX JUMPS OVER THE LAZY DOG"
```

### Input Sanitization
```ghoul
:user_input = String("  JavaScript Alert!  ");
:sanitized = user_input.trim().replace_all("<", "&lt;").replace_all(">", "&gt;");
print sanitized;  # "JavaScript Alert!"
```


## See Also

- [Value Types](./Values.md) - Overview of Ghoul's type system
- [Lists](./List.md) - Working with arrays
- [Standard Library](./StandardLibrary.md) - Built-in functions