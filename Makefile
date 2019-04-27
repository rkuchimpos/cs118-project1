# NAME: Jesse Catalan, Ricardo Kuchimpos
# EMAIL: jessecatalan77@gmail.com, rkuchimpos@gmail.com
# ID: 204785152, 704827423

default:
	gcc -o webserver webserver.c

.PHONY: dist
dist:
	tar -czf 704827423.tar.gz webserver.c report.pdf Makefile README error_404.html bird404.png screenshot.png screen\ shot.png test.txt webpage.html

.PHONY: clean
clean:
	rm -f webserver
	rm -f 704827423.tar.gz
