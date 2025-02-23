fn malloc(d: i32) -> u8^;
fn printf(s: u8^) -> i32;

fn m(b: u8^) -> i32 {

	printf("%d\n", b[0]);
	return 0;
}
