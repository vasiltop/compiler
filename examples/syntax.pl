printf :: (s: ^u8) i32

test3 :: () void {

}

main :: () i32 {
	test("test");
	ret 0;
}

test :: (s: ^u8) void {
	printf("hello world\n");
}

#import "other.pl"
