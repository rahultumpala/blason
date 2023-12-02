output: exec clean

exec:
	@gcc ./src/libjson/json.h ./src/libjson/print.h ./src/libjson/json.c ./src/libjson/print.c ./src/main.c -o output
	@./output

.PHONY: clean

clean:
	@find . -name "*.o" -type f -delete
	@find . -name "*.exe" -type f -delete
	@find . -name "output" -delete
	@find . -name "*.h.gch" -type f -delete