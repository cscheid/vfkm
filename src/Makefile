# FLAGS=-g -Wall # debug
# FLAGS=-O2 -g -pg -Wall # profile
FLAGS=-O3 -Wall # release

emscripten:
	emcc --bind -O2 -o www/vfkm.js main_emscripten.cpp Vector.cpp PolygonalPath.cpp Vector2D.cc Util.cpp Grid.cpp Optimizer.cpp ConstraintMatrix.cpp -I.

main:
	g++ $(FLAGS) -o vfkm main.cpp Vector.cpp PolygonalPath.cpp Vector2D.cc Util.cpp Grid.cpp Optimizer.cpp ConstraintMatrix.cpp -I.

clean:
	rm vfkm
	rm www/vfkm.js
	rm *.o
