fn printf(s: i8^) -> i32;
fn malloc(size: i32) -> i8^;
// a
fn main() -> i32 {
	let str: i8^ = "test\n";
	let a: i32 = 5;
	let b: bool = true;
	printf("Hello: %d, bool: %d\n", a, b);

	return 0;
}