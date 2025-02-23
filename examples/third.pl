#include "std/io.pl";

fn foo() -> void {
    printf("foo\n");
    foo();
}

fn main() -> i32 {
    foo();
    return 0;
}