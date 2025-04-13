module "entry"
import "std:io"

main :: () i32 {
	let a: [[i32; 2]; 2] = [[0, 1], [2, 3]];

	io.print("%d\n", a[1][1]);
	a[1][1] = 420;
	io.print("%d\n", a[1][1]);

	return 0;
}
