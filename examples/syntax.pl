module "entry"
import "std:io"

main :: () i32 {
	let a: i32 = 21321;
	let b: ^i32 = &a;

	io.print("%d\n", ^b);


	return 0;
}
