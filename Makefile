all: opsh.c
	@echo "     CC  opsh"
	@gcc -o opsh opsh.c -O2
	@echo "   DONE  ."
clean:
	@echo "  CLEAN  opsh"
	@rm -f opsh
	@echo "   DONE  ."
install:
	@echo "   COPY  /bin/opsh"
	@cp ./opsh /bin
	@echo "   DONE  /bin"
uninstall:
	@echo "  CLEAN  /bin/opsh"
	@rm /bin/opsh -f
	@echo "   DONE  /bin"
