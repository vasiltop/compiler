module "entry"
import "std:io"

main :: () i32 {
    let bigNum: i32 = 12345678;
		let smallNum: u8 = @cast(u8, bigNum);
    
    io.print("Original i32 value: 0x%x\n", bigNum);
    io.print("Truncated u8 value: 0x%x\n", @cast(u8, bigNum));
    
    let negative: i32 = -42;
    let unsignedSmall: u8 = @cast(u8, negative);
    
    io.print("\nNegative i32 value: %d\n", negative);
    io.print("Cast to u8: %u (0x%x)\n", unsignedSmall, unsignedSmall);

    
    return 0;
}
