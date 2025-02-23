#include "std/io.pl";

struct Test {
    a: i32,
}

fn main() -> i32 {
    let b: Test;
    b.a = 31;

    return b.a;
}

