module "entry"
import "std:io"

main :: () i32 {
	io.print("%d\n", entry.test());

	return 0;
}

test :: () i32 {
	return 5;
}
