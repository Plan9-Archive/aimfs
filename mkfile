</$objtype/mkfile

TARG=oscarlnet

OFILES=\
	flap.$O\
	main.$O\
	snac.$O\
	tlv.$O\
	login.$O\
	ssi.$O\

HFILES=dat.h\
	fns.h\

UPDATE=\
	mkfile\
	$HFILES\
	${OFILES:%.$O=%.c}\

%.$O:	%.c
	$CC $CFLAGS $stem.c

$TARG:	$OFILES $HFILES
	$LD -o $TARG $OFILES

clean:
	rm -f $TARG $OFILES
