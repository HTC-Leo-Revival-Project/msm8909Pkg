all: nbgen

clean:
	@rm -f nbgen

nbgen: nbgen.c
	@echo "[CC] $@"
	@$(CC) -std=c99 $(filter %.c,$^) -o $@
	@chmod a+x $@
