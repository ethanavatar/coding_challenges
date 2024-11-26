CC=clang
CXX=clang++
CXX_FLAGS=-Wall -Wextra -Wpedantic -Wconversion -std=c++20 -O0 -g -fsanitize=address,undefined,integer

OUT_DIR=bin/$(CONFIG)
LINK_FLAGS=-static -m64

INCLUDE_RAYLIB=-Iraylib/src -Lraylib/src -lraylib -lwinmm -lgdi32 -lm

.PHONY: all
all: $(OUT_DIR)/01_starfield.exe

$(OUT_DIR)/01_starfield.exe: raylib |$(OUT_DIR)
	$(CXX) $(CXX_FLAGS) -I. $(INCLUDE_RAYLIB) -o $(OUT_DIR)/01_starfield.exe 01_starfield.cpp $(LINK_FLAGS)

.PHONY: 01_starfield
01_starfield: $(OUT_DIR)/01_starfield.exe
	$(OUT_DIR)/01_starfield.exe

.PHONY: raylib
raylib:
	cd raylib/src && make CC=$(CC) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=STATIC RAYLIB_BUILD_MODE=RELEASE

.PHONY: raylib_clean
raylib_clean:
	cd raylib/src && make clean

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

$(OUT_DIR):
	@echo "Creating output directory: $(OUT_DIR)"
	mkdir -p $(OUT_DIR)
