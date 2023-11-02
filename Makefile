UNAME = $(shell uname)
STRIP ?= strip

CFLAGS ?= -Wall
OPEN ?= xdg-open

trident: trident.c
	$(CC) $(CFLAGS) -m32 -o $@ $<

pub.css:
	wget https://github.com/manuelp/pandoc-stylesheet/raw/acac36b976966f76544176161ba826d519b6f40c/pub.css

README.html: README.md pub.css # Requires Pandoc to be installed
	pandoc README.md -s -c pub.css -o $@
	$(OPEN) $@

strip: trident
	$(STRIP) $^

clean:
	rm trident
