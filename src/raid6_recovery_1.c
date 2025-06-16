#include "gf_raid6.h"

#define BLOCK_SIZE 16

void recover_part1() {
	const char* paths[] = {"../drives/sda", "../drives/sdb", "../drives/sdc",
	                       "../drives/sdd"};

	FILE* sda = fopen(paths[0], "rb");
	FILE* sdb = fopen(paths[1], "rb");
	FILE* sdc = fopen(paths[2], "rb");
	FILE* sdd = fopen(paths[3], "rb");
	FILE* out = fopen("recovered.txt", "w");

	if (!sda || !sdb || !sdc || !sdd || !out) {
		perror("File open error");
		exit(1);
	}

	int count = 0;
	while (1) {
		unsigned char* sda_block = (unsigned char*)malloc(BLOCK_SIZE);
		unsigned char* sdb_block = (unsigned char*)malloc(BLOCK_SIZE);
		unsigned char* sdc_block = (unsigned char*)malloc(BLOCK_SIZE);
		unsigned char* sdd_block = (unsigned char*)malloc(BLOCK_SIZE);

		size_t sda_read = fread(sda_block, 1, BLOCK_SIZE, sda);
		size_t sdb_read = fread(sdb_block, 1, BLOCK_SIZE, sdb);
		size_t sdc_read = fread(sdc_block, 1, BLOCK_SIZE, sdc);
		size_t sdd_read = fread(sdd_block, 1, BLOCK_SIZE, sdd);

		if (sda_read + sdb_read + sdc_read + sdd_read == 0) {
			break;
		}

		if (sda_read < BLOCK_SIZE) {
			free(sda_block);
			sda_block = NULL;
		}
		if (sdb_read < BLOCK_SIZE) {
			free(sdb_block);
			sdb_block = NULL;
		}
		if (sdc_read < BLOCK_SIZE) {
			free(sdc_block);
			sdc_block = NULL;
		}
		if (sdd_read < BLOCK_SIZE) {
			free(sdd_block);
			sdd_block = NULL;
		}

		unsigned char* P_block = NULL;
		unsigned char* Q_block = NULL;
		unsigned char* block1 = NULL;
		unsigned char* block2 = NULL;

		switch (count % 4) {
		case 0:
			P_block = sdd_block;
			Q_block = sda_block;
			block1 = sdb_block;
			block2 = sdc_block;
			break;
		case 1:
			P_block = sdc_block;
			Q_block = sdd_block;
			block1 = sda_block;
			block2 = sdb_block;
			break;
		case 2:
			P_block = sdb_block;
			Q_block = sdc_block;
			block1 = sdd_block;
			block2 = sda_block;
			break;
		case 3:
			P_block = sda_block;
			Q_block = sdb_block;
			block1 = sdc_block;
			block2 = sdd_block;
			break;
		}

		four_drive_recovery(&block1, &block2, P_block, Q_block, BLOCK_SIZE);

		if (block1 && block2) {
			fwrite(block1, 1, BLOCK_SIZE, out);
			fwrite(block2, 1, BLOCK_SIZE, out);
		}

		free(sda_block);
		free(sdb_block);
		free(sdc_block);
		free(sdd_block);
		if (block1 != sda_block && block1 != sdb_block && block1 != sdc_block &&
		    block1 != sdd_block) {
			free(block1);
		}
		if (block2 != sda_block && block2 != sdb_block && block2 != sdc_block &&
		    block2 != sdd_block) {
			free(block2);
		}

		count++;
	}

	fclose(sda);
	fclose(sdb);
	fclose(sdc);
	fclose(sdd);
	fclose(out);
}
