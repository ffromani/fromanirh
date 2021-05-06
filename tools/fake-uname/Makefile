.PHONY: all
all: fake-uname.so

fake-uname.so: fake-uname.c
	gcc -Wall -fPIC -shared -o fake-uname.so fake-uname.c

.PHONY: clean
clean:
	rm fake-uname.so
