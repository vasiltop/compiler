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
message: string = "Hello, world!";
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
    return a + b;
}

main :: () i32 {
    let added: i32 = add(4, 2);
    return 0;
}
```

## Arrays

```rust
main :: () i32 {
    let a: [i32; 2] = [2, 4];

    return 0;
}
```

## Structs
```rust
Other :: struct {
	b: i32
}

Test :: struct {
	b: entry:Other
	other: entry:Other
}

main :: () i32 {
	let a: entry:Test = entry:Test { 
		b: entry:Other { b: 1 }
		other: entry:Other { b: 3 }
	};

	io:print("%d\n", a.b.b);
	io:print("%d\n", a.other.b);

	return 0;
}
```
