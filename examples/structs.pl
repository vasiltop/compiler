module "entry"
import "std:io"

Test :: struct {
	a: i32,
	b: entry:Other
}

Other :: struct {
	b: i32
}

main :: () i32 {
	let a: entry:Test = entry:Test { 
		a: 5,
		b: entry:Other { b: 1 }
	};

	io:print("%d\n", a.b.b);
	a.a = 10;
	io:print("%d\n", a.a);
	return 0;
}
