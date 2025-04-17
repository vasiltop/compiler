module "main"
import "std:io"

main :: () i32 {
	let i: i32 = 0;

	while i < 2147483647 {
		i = i + 1;
	}

	io:print("%d\n", i);

	return 0;
}
