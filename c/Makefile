PROG	= bkpr
MAN	=
SRCS	= main.c\
	  util.c\
	  err.c\
	  create.c\
	  test.c

DB	?= SQLITE

.if $(DB) == SQLITE
SRCS	+= sqlite3.c
CFLAGS	+= -DDB_SQLITE
LDADD	+= -L/usr/local/lib -lsqlite3 -lpthread
.endif

#.if $(DB) == MYSQL
#SRCS	+= mysql.c
#CFLAGS	+= -DDB_MYSQL -I/usr/local/include/mysql
#LDADD	+= -L/usr/local/lib/mysql -lmysqlclient -pthread -lm
#.endif

.include <bsd.prog.mk>
