#include <stdio.h>

#define SECTOR_SIZE 512

const char* filename = "../../disk.img";

#pragma pack(push, 1)

/* 파티션 구조체 타입 선언 */
typedef struct {
    unsigned char boot_flag;
    unsigned char chs_start[3];
    unsigned char type;
    unsigned char chs_end[3];
    unsigned int lba_start;
    unsigned int size_in_sector;
}PARTITION_TABLE;

/* MBR을 모델링하기 위한 구조체 타입 선언 */
typedef struct {
    unsigned char bootcode[446];
    PARTITION_TABLE pt[4];
    unsigned short signature;
}MBR;

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
}BR;

#pragma pack(pop)
/* MBR, BR 분석을 통하여 획득하여 저장해야 하는 정보들 */

static struct _parameter
{
    unsigned int lba_start;
    unsigned short byte_per_sector;
    unsigned char sector_per_cluster;
    unsigned int root_sector_count;
    unsigned int fat0_start;
    unsigned int root_start;
    unsigned int file_start;
}parameter;

/* 하부 설계되는 함수 목록 */
void FAT16_Test(void);
void Sector_Uart_Printf(void * buf);

int main(int argc, char *argv[]) {
    FAT16_Test();
    return 0;
}

void FAT16_Test(void)
{
    unsigned int buf[128] = {0x0,};
    int i;

    printf("\nFAT16 File Read\n");

    Read_Sector(0, (unsigned char *)buf);

    /* MBR에서 첫번째 lba_start 값을 읽어서 parameter 구조체 멤버들에 저장하고 내용 인쇄  */
    parameter.lba_start = ((MBR *)buf)->pt[0].lba_start;


    printf("LBA_Start = %d\n", parameter.lba_start);

    Read_Sector(parameter.lba_start, (unsigned char *)buf);

    /* BR에서 정보들을 읽어서 parameter 구조체 멤버들에 저장하고 내용 인쇄 */
    BR *br = ((BR *)buf);
    parameter.byte_per_sector = br->byte_per_sector;
    parameter.sector_per_cluster = br->sector_per_cluster;
    parameter.fat0_start = parameter.lba_start + br->rsvd_sec_cnt;
    parameter.root_start = parameter.fat0_start + (br->number_of_fat * br->fat16_size);
    parameter.root_sector_count = (br->root_entry_count * 32) / br->byte_per_sector;
    parameter.file_start = parameter.root_start + parameter.root_sector_count;

    printf("=============================================================\n");
    printf("number_of_fat(FAT의 수) = %d\n", br->number_of_fat);
    printf("root_entry_count(Root의 가능한 최대 Entry(file, dir 등) 수) = %d\n", br->root_entry_count);
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

int Read_Sector(int sec, void *buf) {
    int i;
    FILE *imagefile;
    unsigned int* buf_read;
    buf_read = (unsigned int*) buf;
    if ((imagefile  = fopen(filename, "rb")) == NULL) {
        fputs("파일 오픈 에러\n", stderr);
        return 1; // 비주얼C 2003에서의 파일 열기
    }

    if ( fread(buf_read, SECTOR_SIZE, 1, imagefile) != NULL ) {
        Sector_Uart_Printf(buf_read);
    }
}

/* 512B 단위의 섹터를 인쇄해주는 디버깅용 함수 */

void Sector_Uart_Printf(void * buf)
{
    int i, j, k;

    for(i=0; i<(128/8); i++)
    {
        printf("[%3d]", i*32);

        for(j=0; j<8; j++)
        {
            for(k=0; k<4; k++)
            {
                printf("%.2X", ((unsigned char *)buf)[i*32+j*4+k]);
            }
            printf(" ");
        }
        printf("\n");
    }
}