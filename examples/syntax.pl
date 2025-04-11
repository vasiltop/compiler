module "entry"
import "std:io"

main :: () i32 {
	let a: i32 = 5;
	io.print("%d\n", a);
	a = "dsa";
	io.print("%d\n", a);
	return 0;
}
