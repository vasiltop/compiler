# Compiler 

A statically typed, compiled programming language. 

## Installation

```bash
git clone https://github.com/vasiltop/compiler
cd compiler 
make build
```

## Usage

```bash
compiler <PATH>
./a.out
```

## Examples

Code examples can be found in the examples directory.

## Variables

```rust
message: u8^ = "Hello, world!";
```

## Control Flow

```rust
if x == 1 && true {
    x = 4;
} else {
    x = 7;
}

let i: i32 = 0;
while i < 10 {
    i = i + 1;
}
```

## Functions

```rust
add :: (a: i32, b: i32) i32 {
    ret a + b;
}

main :: () i32 {
    added := add(4, 2);
    ret 0;
}
```

## Structs

```rust
Test :: {
    a: i32,
    b: i32
}

main :: () i32 {
    b := Test { a: 1, b: 5 };
    ret b.a;
}
```
