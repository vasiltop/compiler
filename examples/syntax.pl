module "entry"
import "std:io"

main :: () i32 {
	let a: string = "hello world";
	io.print("%c", ^(a ));
	io.print("%c", ^(a + 1));
	io.print("%c", ^(a + 2));
	io.print("%c", ^(a + 3));

	let b: i32 = 4;
	io.print("%c\n", ^(a + b));

	entry.test();

	// let c: u8 = @cast(u8, 4);

	return 0;
}

test :: () void {
	io.print("New line\n");
}
