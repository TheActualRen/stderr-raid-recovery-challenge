# Makefile

OUT = build/main
SRC = src/main.c src/gf_raid6.c src/raid6_recovery_1.c src/raid6_recovery_2.c

all:
	gcc -o $(OUT) $(SRC)
