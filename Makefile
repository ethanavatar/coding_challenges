CC=clang
CXX=clang++
CXX_FLAGS=-Wall -Wextra -Wpedantic -Wconversion -std=c++20 -O0 -g -gcodeview -Wl,--pdb= -fsanitize=address,undefined,integer

OUT_DIR=bin/$(CONFIG)

INCLUDE_RAYLIB=-Iraylib/src -Lraylib/src -lraylib -lwinmm -lgdi32 -lm

.PHONY: all
all: $(OUT_DIR)/coding_challenges.exe

$(OUT_DIR)/01_starfield.dll: raylib |$(OUT_DIR)
	$(CXX) $(CXX_FLAGS) -I. $(INCLUDE_RAYLIB) -o $(OUT_DIR)/01_starfield.dll 01_starfield.cpp -shared -m64 -fPIC

$(OUT_DIR)/02_menger_sponge.dll: raylib |$(OUT_DIR)
	$(CXX) $(CXX_FLAGS) -I. $(INCLUDE_RAYLIB) -o $(OUT_DIR)/02_menger_sponge.dll 02_menger_sponge.cpp -shared -m64 -fPIC

$(OUT_DIR)/coding_challenges.exe:   \
	$(OUT_DIR)/01_starfield.dll     \
	$(OUT_DIR)/02_menger_sponge.dll
	$(CXX) $(CXX_FLAGS) -I. $(INCLUDE_RAYLIB) -o $(OUT_DIR)/coding_challenges.exe main.cpp -m64

.PHONY: run
run: $(OUT_DIR)/coding_challenges.exe
	$(OUT_DIR)/coding_challenges.exe

.PHONY: raylib
raylib: |$(OUT_DIR)
	cd raylib/src && make CC=$(CC) PLATFORM=PLATFORM_DESKTOP RAYLIB_LIBTYPE=SHARED RAYLIB_BUILD_MODE=DEBUG
	cp raylib/src/raylib.dll $(OUT_DIR)/raylib.dll

.PHONY: raylib_clean
raylib_clean:
	cd raylib/src && make clean

.PHONY: clean
clean:
	rm -rf $(OUT_DIR)

$(OUT_DIR):
	@echo "Creating output directory: $(OUT_DIR)"
	mkdir -p $(OUT_DIR)
