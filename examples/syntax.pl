module "entry"
import "other.pl"

printf :: (s: ^u8) i32

main :: () i32 {
	entry.test("test");
	ret 0;
}

test :: (s: ^u8) void {
	entry.printf("main file\n");
	other.test("test");
}

