run-odin:
	mkdir -p target/odin && odin build odin/raycaster.odin -file -out:target/odin/raycaster && ./target/odin/raycaster

run-c:
	mkdir -p target/c && gcc -o target/c/raycaster c/*.c -lSDL3 -lm && ./target/c/raycaster
