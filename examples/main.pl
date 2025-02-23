fn printf(s: u8^) -> i32;
fn malloc(d: i32) -> void^;

struct Test {
	a: i32^,
	b: i32,
}

fn main() -> i32 {
	let t: Test;

	t.a = malloc(8);

	let c: i32^ = t.a;
	c[0] = 69;

	let p: i32^ = t.a;

	let test: i32 = p[0];
	
	printf("%d\n", test);
	return 0;
}