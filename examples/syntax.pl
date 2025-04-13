module "entry"
import "std:io"

main :: () i32 {
	let a: i32 = 0;

	if a == 0 {
		io.print("a is 0\n");
		let b: i32 = 5;
	}	

	io.print("%d\n", b);

	return 0;
}
