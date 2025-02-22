fn printf(s: i8^) -> i32;
fn malloc(size: i32) -> i8^;
// a

fn main() -> i32 {
	let a: u8^ = malloc(8);
	printf("Old Pointer: %d\n", a^);
	a^ = 1;
	printf("New Pointer: %d\n", a^);

	return 0;
}