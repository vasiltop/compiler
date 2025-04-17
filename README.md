# jolt

Jolt is a statically-typed, natively compiled language designed for performance-critical applications where precise memory control is essential. The language provides manual memory management without garbage collection, giving developers complete authority over their code.

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
	b: [i32; 2]
}

Test :: struct {
	b: Other
	other: Other
}

main :: () i32 {
	let a: Test = Test { 
		b: Other { b: [1, 2] }
		other: Other { b: [3, 4] }
	};

	io:print("%d\n", a.b.b[1]);
	io:print("%d\n", a.other.b[1]);
}
```
