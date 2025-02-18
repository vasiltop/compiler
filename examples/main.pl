struct MyStruct {
	int x;
	int y;
};

fn main() -> int {
	let a: MyStruct = MyStruct { x = 1, y = 2 };

	printf("Hello, world!\n");
	printf("a.x = %d\n", a.x);

	for (let i = 0; i < 10; i++) {
		printf("i = %d\n", i);
	}

	while (a.x < 10) {
		printf("a.x = %d\n", a.x);
		a.x = a.x + 1;
	}

	if (a.x == 10) {
		printf("a.x == 10\n");
	} else {
		printf("a.x != 10\n");
	}

	return 0;
}
