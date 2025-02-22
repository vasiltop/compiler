fn printf(s: u8^) -> i32;

fn main() -> i32 {

	let b: bool = true;
	let a: bool = false;

	if b && a {
		printf("true\n");
	} else {
		printf("false\n");
	}

	return 0;
}