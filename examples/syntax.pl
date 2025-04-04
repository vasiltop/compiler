printf :: (s: ^u8) i32

main :: () i32 {
	test("test");
	test2("test2");
	ret 0;
}

#import "./other.pl"

test :: (s: ^u8) void {
	printf("hello world\n");
}
