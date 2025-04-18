module "main"
import "std:io"

Test :: struct {
	a: i32
}

main :: () i32 {
	let test: Test = Test { a: 5 };	
	let other: ^Test = &test;

	let new: Test = ^other;
	io:print("%d\n", (^other).a);
	return 0;
}
