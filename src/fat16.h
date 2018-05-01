/*
 * fat16.h
 *
 *  Created on: 2018. 5. 1.
 *      Author: KyuHyuk Lee
 */

#define SECTOR_SIZE 512

#pragma pack(push, 1)

/* 파티션 구조체 타입 선언 */
typedef struct {
	unsigned char boot_flag;
	unsigned char chs_start[3];
	unsigned char type;
	unsigned char chs_end[3];
	unsigned int lba_start;
	unsigned int size_in_sector;
} PARTITION_TABLE;

/* MBR을 모델링하기 위한 구조체 타입 선언 */
typedef struct {
	unsigned char bootcode[446];
	PARTITION_TABLE pt[4];
	unsigned short signature;
} MBR;

/* BR을 모델링하기 위한 구조체 타입 선언 */
typedef struct {
	unsigned char jump_code;
	unsigned int :32;
	unsigned int :32;
	unsigned short :16;
	unsigned short byte_per_sector; // Sector당 바이트수
	unsigned char sector_per_cluster; // Cluster당 Sector수
	unsigned short rsvd_sec_cnt; // BR을 포함한 reserved 영역의 sector 수
	unsigned char number_of_fat; // FAT의 수
	unsigned short root_entry_count; // Root의 가능한 최대 Entry(file, dir 등) 수
	unsigned short total_sector; // 해당 Volume의 총 크기 (Sector 수)
	unsigned char :8;
	unsigned short fat16_size; // FAT영역의 Sector수
} BR;

#pragma pack(pop)

/* MBR, BR 분석을 통하여 획득하여 저장해야 하는 정보들 */
static struct _parameter {
	unsigned int lba_start;
	unsigned short byte_per_sector;
	unsigned char sector_per_cluster;
	unsigned int root_sector_count;
	unsigned int fat0_start;
	unsigned int root_start;
	unsigned int file_start;
} parameter;

/* 하부 설계되는 함수 목록 */
int readSector(int sector, void *buf);
void printSector(void * buf);
