printf :: (s: ^u8) i32

main :: () i32 {
	test("test");
	// test2()
	ret 0;
}

#import "./other.pl"

test :: (s: ^u8) void {
	printf("hello world");
	test3("a");
}

test3 :: (s: ^u8) void {
	printf("hello world");
	test("a");
}
