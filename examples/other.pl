module "other"
import "syntax.pl"

test :: (s: ^u8) void {
	entry.printf("other file\n");
}
