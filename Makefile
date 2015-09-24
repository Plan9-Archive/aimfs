BIN=oscarlnet
CFLAGS=-g
OFILES=\
	flap.o\
	main.o\
	snac.o\
	tlv.o\
	login.o\
	ssi.o\

HFILES=dat.h\
	fns.h\
	linux.h\

$(BIN):	$(OFILES)
	$(CC) -o $(BIN) $(OFILES) -lcrypto 

clean:
	rm $(BIN) $(OFILES)
