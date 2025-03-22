run-odin:
	mkdir -p target/odin && odin build odin/raycaster.odin -file -out:target/odin/raycaster && ./target/odin/raycaster

run-c:
	mkdir -p target/c && gcc -Wall -Werror -g -o target/c/raycaster c/*.c -lSDL3 -lm && ./target/c/raycaster

run-c-optimized:
	mkdir -p target/c/release && gcc -O3 -ffast-math -o target/c/release/raycaster c/*.c -lSDL3 -lm && ./target/c/release/raycaster

