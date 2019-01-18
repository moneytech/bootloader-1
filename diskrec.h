#define MaxPrimary 4
#define MaxSecondary 32
#define MaxPartitions (MaxPrimary + MaxSecondary)
#define MaxRecord (MaxSecondary + 1)

typedef unsigned long lba32;

typedef struct __attribute__((packed)) {
    unsigned char c, h, s;
} chs_t;

typedef struct __attribute__((packed)) {
    unsigned char   boot;
    chs_t           startCHS;
    unsigned char   type;
    chs_t           endCHS;
    lba32           startLBA;
    lba32           size;
} partitionTBL_t;

union disk_record {
    struct dr_t {    
        char bootstrap[ 446 ];
        partitionTBL_t partTbl[ 4 ];
        unsigned short int  sign;
    } record;
    char image[ sizeof(struct dr_t) ]; 
};