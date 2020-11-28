all: pokemon-red/game.asm

pokemon-red.sym: rom-map.wiki.notags Makefile wiki-to-sym.sh
	cat rom-map.wiki.notags | ./wiki-to-sym.sh > $@

%/game.asm: %.gb %.sym
	./mgbdis.py --output-dir=$* --overwrite --print-hex $<
