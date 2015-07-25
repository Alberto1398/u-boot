#ifndef __HWSC__H__
#define __HWSC__H__

/*task return */
typedef struct
{
    /*! 返回值的地址 */
    int addr;

    /*! 返回值的长度 */
    int length;
}TaskInfo_t;

typedef struct
{
    unsigned char ChipID[8];        /* flash id */
    unsigned char CENum;		    /* flash ce number */
    unsigned char Reserved[3];		/* flash ce number */
}__attribute__ ((packed)) FlashScanInfo_t;




#endif