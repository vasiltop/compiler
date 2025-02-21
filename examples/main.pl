#include "examples/other.pl";

fn main() -> int {
	let a: int = 5;
	let b: bool = false;
	let c: bool = true;
	
	returnBoolean(true, b && c);

	return 0;
}