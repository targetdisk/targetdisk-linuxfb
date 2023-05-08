UNAME = $(shell uname)
STRIP ?= strip

CFLAGS ?= -Wall
ifeq ($(UNAME),Linux)
	OPEN=xdg-open
endif

pub-css:
	-[ ! -e pub.css ] && wget https://github.com/manuelp/pandoc-stylesheet/raw/acac36b976966f76544176161ba826d519b6f40c/pub.css

README: pub-css # Requires Pandoc to be installed
	pandoc README.md -s -c pub.css -o README.html
	$(OPEN) README.html

trident: trident.c
	$(CC) $(CFLAGS) -m32 -o $@ $<

strip: trident
	$(STRIP) $^

clean:
	rm trident
