module "entry"
import "std:io"

main :: () i32 {
	let arr: [[i32; 1]; 5] = [[1], [2], [3], [4], [5]];
	
	io.print("%d\n", arr[0][0]);
	return 0;
}
