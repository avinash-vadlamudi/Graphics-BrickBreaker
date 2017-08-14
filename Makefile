
ans: glad.c ans.cpp
	g++ -o ans ans.cpp glad.c -lGL -lglfw -ldl

clean:
	rm ans
