module "entry"
import "std:io"

Other :: struct {
	b: [i32; 2]
}

Test :: struct {
	b: Other
	other: Other
}

main :: () i32 {
	let a: Test = Test { 
		b: Other { b: [1, 2] }
		other: Other { b: [3, 4] }
	};

	io:print("%d\n", a.b.b[1]);
	io:print("%d\n", a.other.b[1]);

	return 0;
}
