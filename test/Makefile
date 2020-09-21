#
LOCALBASE?=	/usr/local

PROGS=		mydb_test
CFLAGS+=	-I${.CURDIR}/../src \
		-I${LOCALBASE}/include
LDFLAGS+=	-L${.CURDIR}/../src \
		-L${LOCALBASE}/lib/mysql \
		-L${LOCALBASE}/lib
LDADD.mydb_test=-lmariadb -lmydb -lfslbase -lcezqueue -lcezmisc

MAN=
MK_DEBUG_FILES=	no
MAKEOBJDIR= .

# keep this empty otherwise installation fails
install:

test:
	./mydb_test

.include <bsd.progs.mk>
