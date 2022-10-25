objects = logger.o password.o

password : $(objects)
	cc -o password $(objects) -lcrypt -lpthread
password.o : password.c
	cc -c password.c
logger.o : logger.c logger.h
	cc -c logger.c

.PHONY : clean
clean :
	-rm password $(objects)
