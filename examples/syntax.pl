module "entry"
import "std:io"

main :: () i32 {
	let a: bool = true;
	let b: bool = true;

	if false {
		io.print("first\n");	
	} else if b and a {
		io.print("second\n");	
	} else {
		io.print("third\n");	
	}

	return 0;
}
