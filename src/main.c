#include <stdio.h>
#include <stdlib.h>
#include "fat16.h"

const char* filename = "disk.img";

BR *br;

void init(void);
void printBr(void);
void printParameter (void);
void printRootFileList(void);
ENTRY* selectFileById(int id);
void readFile(ENTRY * file, void * data);

int main(void) {
//	FAT16_Test();
	init();
//	printRootFileList();
	int i;
	ENTRY * file;
	int num;
	int r=2;

	for (;;) {

		/* ROOT 분석 & 파일 인쇄 */
		printRootFileList();

		printf("File Number(Exit => 999)? ");
		scanf("%d", &num);

		if (num == 999)
			break;

		/* 원하는 번호의 파일 탐색 */

		file = selectFileById(num);

		if ((file == (ENTRY *) 0)) {
			printf("Nonexistent file.\n");
			continue;
		}

		/* 제대로 탐색되었는지 확인하기 위한 이름 인쇄 */

		for (i = 0; i < 8; i++)
			printf("%c", ((char *) file)[i]);
		printf(".");
		for (i = 8; i < (8 + 3); i++)
			printf("%c", ((char *) file)[i]);
		printf("\n");

		// 여기서부터 해보자

		/* C, TXT, BMP 파일인지 확인 */

//		r = check_file_type(file);
//
//		printf("File Type = %d\n", r);
//
//		if(r != 0)
//		{
			char * data;

			/* malloc으로 읽어올 파일 크기만큼 할당 받음 단, 512바이트의 배수로 받아야 함, 할당 결과가 NULL 포인터이면 다시 파일 리스팅 */
			/* 만약 필요한 크기가 520 바이트라면 필요한 520바이트를 초과하는 512바이트의 배수 즉, 1024 바이트를 받아야 한다	*/
			data = (char *) calloc(1, (file->file_size / sizeof(char)) + 1);
			printf("[DEBUG] malloc size = %d\n", (file->file_size / sizeof(char)) + 1);

			/* file의 크기를 num 변수에 저장, 추후 C, TXT 파일의 내용을 인쇄하기 위한 용도로 사용 */
			num = file->file_size;

			/* 파일 데이터 읽기 */
			readFile(file, data);
			printf("\n");

			switch(r)
			{
				case 1 :
				case 2 :
					for(i = 0; i < num; i++) printf("%c", ((char *)data)[i]);
					break;
				case 3 :
//					Lcd_Clr_Screen();
//					Lcd_Draw_BMP_File_24bpp(0,0,(void *)data);
					break;
				default :
					break;
			}

			free(data);
//		}
//		else printf("Not Supported File\n");
	}

	printf("BYE!\n");
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

void init(void) {
	unsigned int buf[128] = { 0x0, };
	readSector(0, (unsigned char *) buf);

	/* MBR에서 첫번째 lba_start 값을 읽어서 parameter 구조체 멤버들에 저장하고 내용 인쇄  */
	parameter.lba_start = ((MBR *) buf)->pt[0].lba_start;

	printf("LBA_Start = %d\n", parameter.lba_start);

	readSector(parameter.lba_start, (unsigned char *) buf);

	/* BR에서 정보들을 읽어서 parameter 구조체 멤버들에 저장하고 내용 인쇄 */
	br = ((BR *) buf);
	parameter.byte_per_sector = br->byte_per_sector;
	parameter.sector_per_cluster = br->sector_per_cluster;
	parameter.fat0_start = parameter.lba_start + br->rsvd_sec_cnt;
	parameter.root_start = parameter.fat0_start
			+ (br->number_of_fat * br->fat16_size);
	parameter.root_sector_count = (br->root_entry_count * 32)
			/ br->byte_per_sector;
	parameter.file_start = parameter.root_start + parameter.root_sector_count;
}

void printBr(void) {
	printf("=============================================================\n");
	printf("number_of_fat(FAT의 수) = %d\n", br->number_of_fat);
	printf("root_entry_count(Root의 가능한 최대 Entry(file, dir 등) 수) = %d\n",
			br->root_entry_count);
	printf("total_sector(해당 Volume의 총 크기 (Sector 수)) = %d\n", br->total_sector);
	printf("fat16_size(FAT영역의 Sector수) = %d\n", br->fat16_size);
	printf("=============================================================\n");
}

void printParameter (void) {
	printf("=============================================================\n");
	printf("byte_per_sector = %d\n", parameter.byte_per_sector);
	printf("sector_per_cluster = %d\n", parameter.sector_per_cluster);
	printf("fat0_start = %d\n", parameter.fat0_start);
	printf("root_start = %d\n", parameter.root_start);
	printf("root_sector_count = %d\n", parameter.root_sector_count);
	printf("file_start = %d\n", parameter.file_start);
	printf("=============================================================\n");
}

void printRootFileList(void) {
	unsigned int buf[128] = { 0x0, };
	int i, j, num = 1, filename_idx;
	/* 타이틀 인쇄 => 임의 변경 가능 */

	printf("[NUM] [  NAME  ] [.EXT] [AT] [  DATE  ] [ TIME ] [CLUST] [  SIZE  ]\n");
	printf("===================================================================\n");

	for (i = 0; i < parameter.root_sector_count; i++) {
		readSector(parameter.root_start + i, (unsigned char *) buf);
		ENTRY *entry = ((ENTRY *) buf);
		for (j = 0; j < (parameter.byte_per_sector / 32); j++) {
			/* Name[0]가 0x0이면 인쇄 종료, 0x05, 0xE5이면 삭제파일 Skip */
			/* 파일 속성이 0x3F 또는 0x0F long file name 이므로 Skip */
			/* Entry 정보 인쇄 */
			/* 인쇄되는 파일 또는 폴더 마다 맨 앞에 1번부터 1씩 증가하며 번호를 인쇄한다 */

			if ((entry + j)->name[0] == 0x00)
				break;
			if ((entry + j)->name[0] == 0xE5 || (entry + j)->name[0] == 0x05)
				continue;
			if ((entry + j)->attribute == 0x3F || (entry + j)->attribute == 0x0F)
				continue;

			printf("[%03d]  ", num++); // 파일 번호 출력
			/* 파일 이름 출력 */
			for (filename_idx = 0; filename_idx < 8; filename_idx++) {
				if ((entry + j)->name[filename_idx] == 0x20)
					printf(" ");
				else
					printf("%c", (entry + j)->name[filename_idx]);
			}
			/* 파일 확장자 출력 */
			printf("   .");
			for (filename_idx = 8; filename_idx < 11; filename_idx++) {
				if ((entry + j)->name[filename_idx] == 0x20)
					printf(" ");
				else
					printf("%c", (entry + j)->name[filename_idx]);
			}
			/* 속성 출력 */
			printf("  %#x", (entry + j)->attribute);

			/* 날짜 출력 */
			DATE * date = (DATE *) (&(entry + j)->creation_date);
			printf(" %04d:%02d:%02d", date->year + 1980, date->month,
					date->day);

			/* 시간 출력 */
			TIME * time = (TIME *) (&(entry + j)->creation_time);
			printf(" %02d:%02d:%02d", time->hour, time->min, time->sec * 2);

			/* CLUSTER LOW 출력 */
			printf(" %6d", (entry + j)->first_cluster_low);

			/* 파일 크기 출력 */
			printf("   %8d", (entry + j)->file_size);
			printf("\n");
		}
	}
	printf("===================================================================\n");
}

ENTRY* selectFileById(int id) {
	unsigned int buf[128] = { 0x0, };
	int i, j, num = 1;

	for(i = 0; i < parameter.root_sector_count; i++)
	{
		readSector(parameter.root_start + i, (unsigned char *)buf);
		ENTRY *entry = ((ENTRY *)buf);
		for(j = 0; j < (parameter.byte_per_sector / 32); j++)
		{
			/* Name[0]가 0x0이면 NULL 포인터 리턴, 0x05, 0xE5이면 Skip */
			/* 파일 속성이 0x3F 또는 0x0F long file name 이므로 Skip */
			/* 정상 Entry(파일, 폴더)일 경우 num이 원하는 ID인지 확인 */
			/* num이 id보다 작으면 num 증가만 하고 계속 탐색 반복, 같으면 Entry 시작 주소 리턴 */
			if((entry + j)->name[0] == 0x00)
				break;
			if((entry + j)->name[0] == 0xE5 || (entry + j)->name[0] == 0x05)
				continue;
			if((entry + j)->attribute == 0x3F || (entry + j)->attribute == 0x0F)
				continue;
			if(id < num)
				break;
			if(id == num)
				return (ENTRY *)(entry+j);
			num++;
		}
	}

	return (ENTRY *)0;
}

void readFile(ENTRY * file, void * data) {
	unsigned short cluster = file->first_cluster_low;
	unsigned short next_cluster = cluster;
	unsigned short cluster_iterator; // 다음 Cluster을 가르키기 위한 단위
	unsigned short cluster_offset;
	unsigned short cluster_buf[256]; // Cluster들을 저장하는 공간
	unsigned char buf[512]; // Cluster에서 받아온 데이터를 저장하는 임시 공간
	unsigned int sector_number = file->file_size / parameter.byte_per_sector;
	unsigned int remaining_sector_size = file->file_size % parameter.byte_per_sector;
	if (remaining_sector_size > 0) {
		sector_number = sector_number + 1;
	}

	printf("[DEBUG] File Info\n");
	printf("file->name : %s\n", file->name);
	printf("file->attribute : %X\n", file->attribute);
	printf("file->creation_time_tenth : %X\n", file->creation_time_tenth);
	printf("file->creation_time : %d\n", file->creation_time);
	printf("file->creation_date : %d\n", file->creation_date);
	printf("file->last_access_date : %d\n", file->last_access_date);
	printf("file->first_cluster_high : %d\n", file->first_cluster_high);
	printf("file->lash_write_time : %d\n", file->lash_write_time);
	printf("file->last_write_date : %d\n", file->last_write_date);
	printf("file->first_cluster_low : %d\n", file->first_cluster_low);
	printf("file->file_size : %d\n", file->file_size);

	int i, j;
	int sector_idx=0, data_idx = 0;
	while ( cluster != 0xFFFF ) {
		/* next_cluster를 갱신 */
		if (next_cluster != 0xFFFF)	{
			cluster = next_cluster;
			cluster_iterator = cluster / FAT_NUM_PER_SECTOR;
			cluster_offset = cluster % FAT_NUM_PER_SECTOR;
			readSector(parameter.fat0_start + cluster_iterator, (unsigned char *) cluster_buf);
			next_cluster = cluster_buf[cluster_offset];
		}

		for (i = 0; i < parameter.sector_per_cluster; i++) {
			readSector(parameter.file_start + (parameter.sector_per_cluster * (cluster - 2)) + i, (unsigned char *) buf);
			/* 마지막 Cluster일 경우에는 남은 공간 모두를 읽어 온다. */
			if (sector_idx == sector_number) {
				for (j = 0; j < remaining_sector_size; j++) {
//					((unsigned char*) data)[data_idx++] = buf[j];
					printf("%c", buf[j]);
				}
				return;
			}

			else {
				for (j = 0; j < parameter.byte_per_sector; j++) {
//					((unsigned char*) data)[data_idx++] = buf[j];
					printf("%c", buf[j]);
				}
			}
			sector_idx = sector_idx + 1;
		}
	}
}
