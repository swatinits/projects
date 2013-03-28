CC = g++

TARGET = worker request server 

LIBS =  -lprotobuf-c  -lpthread -lcrypto

all: $(TARGET)

$(TARGET): lsp.o lspmessage.pb-c.o
	$(CC) -o $@ $@.cpp $^ $(LIBS)

%.o:	%.c
	$(CC) -c -o $@ $<

clean:
	rm -f *.o 
	rm -f $(TARGET)

