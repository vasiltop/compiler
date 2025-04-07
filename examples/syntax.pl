module "entry"
import "std:io"

main :: () i32 {
	io.print(("a" + "b") * "c");

	return 0;
}
