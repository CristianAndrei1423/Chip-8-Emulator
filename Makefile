files = main.c chip8.c

.PHONY: clean

main: $(files)
	gcc $(files) -o main `sdl2-config --cflags --libs`

clean:
	rm main