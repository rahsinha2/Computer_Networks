CC=gcc
CPFLAGS=-g -Wall
LDFLAGS= -lpcap

SRC= wiretap.c wiretap_functions.c 
OBJ=$(SRC:.c=.o)
BIN=wiretap

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CPFLAGS) $(LDFLAGS) -o $(BIN) $(OBJ) 


%.o:%.c
	$(CC) -c $(CPFLAGS) -o $@ $<  

$(SRC):

clean:
	rm -rf $(OBJ) $(BIN)
