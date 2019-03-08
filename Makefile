CXXFLAGS := -std=c++1z -Wall -Wextra -pedantic
ICNLUDE_PATH := -I /usr/local/include -I /usr/include
SRC := src/main.cpp


all:
	em++ $(CXXFLAGS) $(INCLUDE_PATH) -DNDEBUG $(SRC)\
		-s WASM=1\
		-s USE_SDL=2\
		-s USE_SDL_TTF=2\
		-s USE_SDL_IMAGE=2\
		-s ASYNCIFY=1\
		--preload-file assets\
		--use-preload-plugins\
		-O2\
		-o parabsbc.js

app:
	g++ $(CXXFLAGS) $(INCLUDE_PATH) -DNDEBUG $(SRC)\
		-lSDL2\
		-lSDL2_ttf\
		-lSDL2_image\
		-O2\
		-o app

debug:
	clang++ $(SRC)\
		-fsanitize=address\
		-fno-omit-frame-pointer\
		$(CXXFLAGS)\
		$(INCLUDE_PATH)\
		-lSDL2\
		-lSDL2_ttf\
		-lSDL2_image\
		-O0\
		-o app

serve:
	@python -m SimpleHTTPServer 8080

clean:
	@rm -rf ./parabsbc.js ./parabsbc.wasm ./parabsbc.data ./index.data ./app ./app.dSYM

.PHONY: all browse
