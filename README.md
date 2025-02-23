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
clang -o out output.ll
```

## Examples

Code examples can be found in the examples directory.

## Variables

```rust
let message: u8^ = "Hello, world!";
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
fn add(a: i32, b: i32) -> i32 {
    return a + b;
}

fn main() -> i32 {
    let added: i32 = add(4, 2);
    return 0;
}
```

## Structs
```rust
struct Test {
    a: i32,
}

fn main() -> i32 {
    let b: Test;

    b.a = 30;

    return b.a;
}
```

## Memory

```rust
#include "std/memory.pl";

fn main() -> i32 {
    let a: i32^ = malloc(8);

    a[0] = 5;
    a[1] = 10;

    return a[0];
}
```