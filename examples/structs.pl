module "entry"
import "std:io"

Other :: struct {
	b: i32
}

Test :: struct {
	b: entry:Other
	other: entry:Other
}

main :: () i32 {
	let a: entry:Test = entry:Test { 
		b: entry:Other { b: 1 }
		other: entry:Other { b: 3 }
	};

	io:print("%d\n", a.b.b);
	io:print("%d\n", a.other.b);

	return 0;
}
