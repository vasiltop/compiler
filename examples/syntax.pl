module "entry"
import "std:io"

main :: () i32 {
	let a: [[i32; 2]; 2] = [[0, 1], [2, 3]];

	let i: i32 = 0;

	while i < 2 {
		io.print("%d\n", a[i][i]);
		i = i + 1;
	}

	return 0;
}
