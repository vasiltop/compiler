#include "std/memory.pl";

fn main() -> i32 {
    let a: i32^ = malloc(8);

    a[0] = 5;
    a[1] = 10;

    return a[1];
}