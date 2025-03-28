#import "./other.pl"

printf :: (s: ^u8) i32

main :: () i32 {
	test("test");
	// test2()
	ret 0;
}

test :: (s: ^u8) void {
	printf("hello world");
}

