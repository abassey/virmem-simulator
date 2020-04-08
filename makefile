all: virmem

virmem: virmem.c
	gcc virmem.c -o virmem
	
clean:
	rm -f virmem
