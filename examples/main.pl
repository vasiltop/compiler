#include "examples/other.pl";

fn malloc(d: i32) -> u8^;

fn main() -> i32 {

	let mlc: u8^ = malloc(4);
	mlc[0] = 4;
	m(mlc);

	return 0;
}