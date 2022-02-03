all: wc

wc: wc_multi.c wc.h wc_core.c
	gcc wc_multi.c wc_core.c -g -o wc

clean:
	rm -f wc 
