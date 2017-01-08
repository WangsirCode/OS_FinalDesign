all : multisum

multisum : thread.c
	gcc -pthread -o multisum thread.c 

clean :
	rm multisum output.txt
