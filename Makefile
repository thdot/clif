
test: test.c console_parser.h
	gcc -save-temps test.c -o test
#	tail -50 test.i
	./test 
	
	
clean:
	rm -f test