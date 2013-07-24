test : serialconn.o
	@cc -o test serialconn.o
serialconn.o : serialconn.c serialconn.h
	@cc -c serialconn.c
clean :
	@rm test
	@echo [CLEAN]
