module "entry"
import "std:io"

Other :: struct {
	b: [i32; 2]
}

Test :: struct {
	b: entry:Other
	other: entry:Other
}

main :: () i32 {
	let a: entry:Test = entry:Test { 
		b: entry:Other { b: [1, 2] }
		other: entry:Other { b: [3, 4] }
	};

	io:print("%d\n", a.b.b[1]);
	io:print("%d\n", a.other.b[1]);

	return 0;
}
