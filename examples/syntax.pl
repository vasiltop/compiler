module "entry"
import "std:io"

main :: () i32 {
	let a: i32 = 21321;
	let b: ^i32 = &a;
	
	^b = 5;
	io.print("%d\n", a);

	^b = 7;
	io.print("%d\n", ^b);
	io.print("%d\n", a);

	return 0;
}
