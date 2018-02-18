STAR_BATTLE_C_FLAGS=-O2 -Wextra -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winline -Wlong-long -Wmissing-prototypes -Wmissing-declarations -Wnested-externs -Wno-import -Wpointer-arith -Wredundant-decls -Wreturn-type -Wshadow -Wstrict-prototypes -Wswitch -Wwrite-strings

star_battle: star_battle.o
	gcc -o star_battle star_battle.o

star_battle.o: star_battle.c star_battle.make
	gcc -c ${STAR_BATTLE_C_FLAGS} -o star_battle.o star_battle.c

clean:
	rm -f star_battle star_battle.o
