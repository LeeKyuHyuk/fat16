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

/* 시간 포맷을 위한 비트필드 구조체 선언 */
typedef struct {
	unsigned short sec :5;
	unsigned short min :6;
	unsigned short hour :5;
} TIME;

/* 날짜 포맷을 위한 비트필드 구조체 선언 */
typedef struct {
	unsigned short day :5;
	unsigned short month :4;
	unsigned short year :7;
} DATE;

/* 하나의 32B Entry 분석을 위한 구조체 => 아래 구조체는 이름을 바꾸지 말고 내용만 채워서 사용할 것 */

typedef struct {
	/* Entry의 각 멤버를 설계한다 */
	unsigned char name[11];
	unsigned char attribute;
	unsigned char reserved;
	unsigned char creation_time_tenth;
	unsigned short creation_time;
	unsigned short creation_date;
	unsigned short last_access_date;
	unsigned short first_cluster_high;
	unsigned short lash_write_time;
	unsigned short last_write_date;
	unsigned short first_cluster_low;
	unsigned int file_size;

} ENTRY;

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

#define FAT_NUM_PER_SECTOR	(parameter.byte_per_sector/2)

/* 하부 설계되는 함수 목록 */
int readSector(int sector, void *buf);
void printSector(void * buf);
