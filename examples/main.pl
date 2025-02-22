fn printf(s: i8^) -> i32;
fn malloc(size: i32) -> i8^;
// a

fn main() -> i32 {
	let a: i32 = 5; 
	a = a + 1;
	printf("Hello: %d\n", a);

	return 0;
}