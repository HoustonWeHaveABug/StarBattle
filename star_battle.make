STAR_BATTLE_C_FLAGS=-c -O2 -Wall -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wshadow -Wstrict-prototypes -Wwrite-strings

star_battle: star_battle.o
	gcc -o star_battle star_battle.o

star_battle.o: star_battle.c star_battle.make
	gcc ${STAR_BATTLE_C_FLAGS} -o star_battle.o star_battle.c

clean:
	rm -f star_battle star_battle.o
