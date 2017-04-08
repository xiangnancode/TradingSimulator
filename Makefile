SOURCE = simulator
CC = g++ -std=c++0x
#Ubuntu
#CFLAGS = -c -I/usr/include/curl 
#LIBS = -L/usr/lib/libcurl -lcurl
#MAC
CFLAGS = -c -I/usr/local/opt/curl/lib
LIBS = -L/usr/local/opt/curl/lib -lcurl
OBJ = $(SOURCE).o
EXCUTE = $(SOURCE)

all: 
	$(CC) $(CFLAGS) $(SOURCE).cpp
	$(CC) -o $(EXCUTE) $(OBJ) $(LIBS)

clean:
	rm -f $(EXCUTE) *.o
