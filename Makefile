
test: test.c console_parser.h
	gcc -save-temps -g test.c -o test
	./test 
	
test.i: test.c console_parser.h
	gcc -E test.c -o test.i
	tail -50 test.i
	
clean:
	rm -f test test.i