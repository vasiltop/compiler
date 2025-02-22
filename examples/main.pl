fn printf(s: u8^) -> i32;

fn main() -> i32 {

	let b: bool = true;
	let a: bool = false;

	if b && a {
		printf("true\n");
	} else {
		printf("false\n");
	}

	let i: i32 = 0;

	while i < 10 {
		printf("Num: %d\n", i);
		i = i + 1;
	}

	return 0;
}