module "entry"
import "std:io"

main :: () i32 {
	let a: i32 = 21321;
	let b: ^i32 = &a;
	a = 5;

	io.print("%d\n", ^b);

	return 0;
}
