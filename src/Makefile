#
MK_PROFILE=	no
LOCALBASE?=	/usr/local
LIB=		mydb
LIBDIR=		${LOCALBASE}/lib
INCSDIR=	${LOCALBASE}/include
CFLAGS+=	-Wall -Wstrict-prototypes -I${.CURDIR} \
		-I${LOCALBASE}/include
SRCS=		mydb.c mydb.h
INCS=		mydb.h
NO_OBJ=		yes

.include <bsd.lib.mk>
