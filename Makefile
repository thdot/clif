
UNITY_ROOT = ../Unity

WARNING_FLAGS += -Wall
#WARNING_FLAGS += -Wextra
WARNING_FLAGS += -Werror 
#WARNING_FLAGS += -Wpointer-arith
#WARNING_FLAGS += -Wcast-align
#WARNING_FLAGS += -Wwrite-strings
#WARNING_FLAGS += -Wswitch-default
#WARNING_FLAGS += -Wunreachable-code
#WARNING_FLAGS += -Winit-self
#WARNING_FLAGS += -Wmissing-field-initializers
#WARNING_FLAGS += -Wno-unknown-pragmas
#WARNING_FLAGS += -Wstrict-prototypes
#WARNING_FLAGS += -Wundef
#WARNING_FLAGS += -Wold-style-definition
#WARNING_FLAGS += -Wmissing-prototypes
#WARNING_FLAGS += -Wmissing-declarations

CFLAGS = -g

INC_DIRS = -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src

UNITY_SOURCSES = $(UNITY_ROOT)/src/unity.c $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c 

clif.test.i: clif.c clif.h Makefile clif.test.c
	clang -E $(CFLAGS) $(INC_DIRS) clif.test.c -o clif.test.i

clif.test: clif.c clif.h Makefile clif.test.c
	clang -save-temps $(CFLAGS) $(WARNING_FLAGS) $(INC_DIRS) $(UNITY_SOURCSES) clif.c clif.test.c -o clif.test 
	./clif.test -v
	
clean:
	rm -f clif.test *.s *.i
	