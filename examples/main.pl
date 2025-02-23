#include "std/memory.pl";
#include "std/io.pl";
#include "std/sdl.pl";

fn main() -> i32 {
	let SDL_INIT_VIDEO: i32 = 32;
	let SDL_QUIT: i32 = 256;
	
	let window: void^ = SDL_CreateWindow("Hello World!", 100, 100, 640, 480, 0);
	let renderer: void^ = SDL_CreateRenderer(window, -1, 0);
	
	let running: bool = true;
	let event: i32^ = malloc(56);

	while running {
		while SDL_PollEvent(event) != 0 {
           if event[0] == SDL_QUIT {
               running = 0;
            }
        }

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

		let rect: i32^ = malloc(16);
		rect[0] = 100;
        rect[1] = 100;
        rect[2] = 200;
        rect[3] = 200;

        SDL_RenderFillRect(renderer, rect);
		SDL_RenderPresent(renderer);
		free(rect);
	}

	return 0;
}