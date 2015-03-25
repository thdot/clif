
test: test.c clif.h
	clang -save-temps -g test.c -o test
	./test 
	
test.i: test.c clif.h
	clang -E test.c -o test.i
	tail -50 test.i
	
clean:
	rm -f test test.i