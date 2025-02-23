fn printf(s: u8^) -> i32;
fn malloc(d: i32) -> void^;

struct Test {
	a: i32^,
	b: i32,
}

fn main() -> i32 {
	let t: Test;

	t.a = malloc(4);

	let ptr: i32^ = t.a;
	ptr[0] = 10;


	t.b = 10;
	printf("%d\n", t.b);

	t.b = 12;
	printf("%p\n", t.a);


	return 0;
}