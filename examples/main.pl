#include "examples/other.pl";

fn malloc(b: i32) -> u8^;
fn printf(s: u8^) -> i32;

fn main() -> i32 {
	let b: i32^ = m(4);
	printf("%d\n", b[0]);

	return 0;
}

