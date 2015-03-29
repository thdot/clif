
UNITY_ROOT=../Unity

CFLAGS += -Wall
#CFLAGS += -Wextra
CFLAGS += -Werror 
#CFLAGS += -Wpointer-arith
#CFLAGS += -Wcast-align
#CFLAGS += -Wwrite-strings
#CFLAGS += -Wswitch-default
#CFLAGS += -Wunreachable-code
#CFLAGS += -Winit-self
#CFLAGS += -Wmissing-field-initializers
#CFLAGS += -Wno-unknown-pragmas
#CFLAGS += -Wstrict-prototypes
#CFLAGS += -Wundef
#CFLAGS += -Wold-style-definition
#CFLAGS += -Wmissing-prototypes
#CFLAGS += -Wmissing-declarations

INC_DIRS=-I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src

UNITY_SOURCSES=$(UNITY_ROOT)/src/unity.c $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c 


clif.test: clif.c clif.h Makefile clif.test.c
	gcc -save-temps $(CFLAGS) $(INC_DIRS) $(UNITY_SOURCSES) clif.c clif.test.c -o clif.test 
	./clif.test -v
	
clean:
	rm -f clif.test *.s *.i
	