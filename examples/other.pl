#include "examples/third.pl";

fn foo(a: i32, b: i32) -> i32 {
	return a + b + third();
}