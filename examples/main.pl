fn malloc(b: i32) -> u8^;
fn printf(s: u8^) -> i32;

fn main() -> i32 {
	let a: i32^ = malloc(4 * 8);
	
	let i: i32 = 0;

	while i < 4 {
		a[i] = i;
		i = i + 1;
	}

	i = 0;	
	let sum: i32 = 0;
	while i < 4 {
		sum = sum + a[i];
		i = i + 1;
	}

	printf("%d\n",  sum);	

	return 0;
}