all: yang

clean:
	@rm -f yang

yang: *.c *.h
	@echo "[CC] $@"
	@$(CC) $(filter %.c,$^) -o $@
	@chmod a+x $@
