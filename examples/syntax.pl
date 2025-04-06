printf :: (s: ^u8) i32

#import "other.pl"

test :: (s: ^u8) void {
	printf("hello world\n");
}

main :: () i32 {
	test("test");
	test2("test2");
	ret 0;
}



