PROG 		= jngram
SRC 		= ${PROG}.c
OBJ 		= ${SRC:.c=.o}

CC 		= gcc
INCS 		= -I/usr/include/json-c
LIBS 		= -ljson-c

LDFLAGS = ${LIBS}
CFLAGS 	= -Wall -Wextra -O0 ${INCS}

all: ${PROG}

${PROG}: ${OBJ}
	${CC} -o $@ ${LDFLAGS} ${OBJ}

%.o: %.c
	${CC} -c $< ${CFLAGS}

clean:
	rm ${OBJ} ${PROG}

.PHONY: all clean
