module "entry"
import "std:io"

main :: () i32 {
	let c: char = 'c';

	io.print("%c\n", c);

	return 0;
}
