#import "./other.pl"

printf :: (s: ^u8) i32

main :: () i32 {
	test();
	// test2()
	ret 0;
}

test :: () void {
	printf("hello world");
}

