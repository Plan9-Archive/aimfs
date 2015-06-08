</$objtype/mkfile

TARG=aimfs

OFILES=\
	flap.$O\
	main.$O\
	snac.$O\
	tlv.$O\
	login.$O\
	fs.$O\

HFILES=dat.h\
	fns.h\

UPDATE=\
	mkfile\
	$HFILES\
	${OFILES:%.$O=%.c}\

%.$O:	%.c
	$CC $CFLAGS $stem.c

$TARG:	$OFILES
	$LD -o $TARG $OFILES

clean:
	rm $TARG $OFILES
