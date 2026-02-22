all: opsh.c
	@echo "     CC  opsh"
	@gcc -o opsh opsh.c -O2
clean:
	@echo "  CLEAN  opsh"
	@rm -f opsh
	@echo "   DONE  ."
