fn SDL_Init(flags: i32) -> i32;
fn SDL_CreateWindow(title: u8^, x: i32, y: i32, w: i32, h: i32, flags: i32) -> void^;
fn SDL_CreateRenderer(window: void^, index: i32, flags: i32) -> void^;
fn SDL_DestroyRenderer(renderer: void^) -> void;
fn SDL_DestroyWindow(window: void^) -> void;
fn SDL_Quit() -> void;
fn SDL_PollEvent(event: void^) -> i32;
fn SDL_SetRenderDrawColor(renderer: void^, r: i32, g: i32, b: i32, a: i32) -> i32;
fn SDL_RenderClear(renderer: void^) -> i32;
fn SDL_RenderPresent(renderer: void^) -> void;
fn SDL_RenderFillRect(renderer: void^, rect: void^) -> i32;