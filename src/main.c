#include <stdio.h>
#include <stdlib.h>
#include "fat16.h"

const char* filename = "disk.img";

void FAT16_Test(void);

int main(void) {
//	unsigned int buf[128] = { 0x0, };
//	readSector(0, buf);
//	printSector(buf);
	FAT16_Test();
	return 0;
}

int readSector(int sector, void *buf) {
	FILE *fp;
	unsigned int* buf_read;
	buf_read = (unsigned int*) buf;

	fp = fopen("./disk.img", "rb");
	fseek(fp, SECTOR_SIZE * sector, SEEK_SET);
	fread(buf_read, SECTOR_SIZE, 1, fp);

	fclose(fp);
	return 0;
}

void printSector(void * buf) {
	int i, j, k;
	for (i = 0; i < (128 / 8); i++) {
		printf("[%3d] ", i * 32);
		for (j = 0; j < 8; j++) {
			for (k = 0; k < 4; k++) {
				printf("%.2X", ((unsigned char *) buf)[i * 32 + j * 4 + k]);
			}
			printf(" ");
		}
		printf("\n");
	}
}

void FAT16_Test(void) {
	unsigned int buf[128] = { 0x0, };

	printf("\nFAT16 File Read\n");

	readSector(0, (unsigned char *) buf);

	/* MBR에서 첫번째 lba_start 값을 읽어서 parameter 구조체 멤버들에 저장하고 내용 인쇄  */
	parameter.lba_start = ((MBR *) buf)->pt[0].lba_start;

	printf("LBA_Start = %d\n", parameter.lba_start);

	readSector(parameter.lba_start, (unsigned char *) buf);

	/* BR에서 정보들을 읽어서 parameter 구조체 멤버들에 저장하고 내용 인쇄 */
	BR *br = ((BR *) buf);
	parameter.byte_per_sector = br->byte_per_sector;
	parameter.sector_per_cluster = br->sector_per_cluster;
	parameter.fat0_start = parameter.lba_start + br->rsvd_sec_cnt;
	parameter.root_start = parameter.fat0_start
			+ (br->number_of_fat * br->fat16_size);
	parameter.root_sector_count = (br->root_entry_count * 32)
			/ br->byte_per_sector;
	parameter.file_start = parameter.root_start + parameter.root_sector_count;

	printf("=============================================================\n");
	printf("number_of_fat(FAT의 수) = %d\n", br->number_of_fat);
	printf("root_entry_count(Root의 가능한 최대 Entry(file, dir 등) 수) = %d\n",
			br->root_entry_count);
	printf("total_sector(해당 Volume의 총 크기 (Sector 수)) = %d\n", br->total_sector);
	printf("fat16_size(FAT영역의 Sector수) = %d\n", br->fat16_size);
	printf("=============================================================\n");
	printf("byte_per_sector = %d\n", parameter.byte_per_sector);
	printf("sector_per_cluster = %d\n", parameter.sector_per_cluster);
	printf("fat0_start = %d\n", parameter.fat0_start);
	printf("root_start = %d\n", parameter.root_start);
	printf("root_sector_count = %d\n", parameter.root_sector_count);
	printf("file_start = %d\n", parameter.file_start);
	printf("=============================================================\n");
}
