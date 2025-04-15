module "entry"
import "std:io"

Test :: struct {
	a: [i32; 1],
	b: i32
}

main :: () i32 {
	let a: entry:Test = entry:Test { a: [1], b: 5 };

	io:print("%d\n", a.a[0]);
	a.a = 5;
	return 0;
}
