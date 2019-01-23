struct table_t
{
    char cmd_line[ 118 ];               
    unsigned short stage3_size;         
    unsigned short kernel_size;
    unsigned short initrd_size; // initrd_size param 
    unsigned short mount_flag;                                        
    unsigned short root_disk;             
    unsigned short root_dev;                                              
} __attribute__((packed));
