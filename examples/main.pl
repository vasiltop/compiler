fn malloc(b: i32) -> u8^;
fn printf(s: u8^) -> i32;

fn main() -> i32 {
	let b: i32^ = malloc(4);
	let a: i32 = 42;
	b[0] = 42;

	a = 2;

	printf("%d\n", b[0]);
	printf("%d\n", a);


	return 0;
}

