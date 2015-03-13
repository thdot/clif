
test: test.c console_parser.h
	gcc -save-temps test.c -o test
	./test
	
clean:
	rm -f test