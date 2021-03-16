/* THE VFS WORK ONLY WITH ARDUINO */

#define MAX_OPEN_FILES  4

#define VFS_MINIMAL
#ifdef VFS_MINIMAL ///////////////////////////////////////////////////////////////////////////

#define MAX_OPEN_FILES  4
#define USE_LFS         /* Enable littlefs                 */
#define USE_LFS_RAM     /* Use Ram disk                    */
#define USE_LFS_ROM     /* Use Rom disk ( internal flash ) */
//#define USE_FATFS     /* Enable FatFS */

#else //////////////////////////////////////////////////////////////////////////////////////////

#define USE_LFS         /* Enable   littlefs < https://github.com/littlefs-project/littlefs >   */
#define USE_LFS_RAM     /* use Ram disk                                                         */
#define USE_LFS_ROM     /* use Rom disk (internal flash)                                        */

#ifdef USE_LFS

/* RAM SETTINGS *******************************************************/
#ifdef USE_LFS_RAM
#   define LFS_RAM_LETTER           "R:"
#   define LFS_RAM_BLOCK_COUNT      128
#   define LFS_RAM_BLOCK_SIZE       128
#   define LFS_RAM_PROG_SIZE        16
#   define LFS_RAM_READ_SIZE        16
#   define LFS_RAM_CACHE_SIZE       16
#   define LFS_RAM_LOOKAHEAD_SIZE   16
#   define LFS_RAM_BLOCK_CYCLES     1000
#endif /* USE_LFS_RAM_FS **********************************************/


/* FLASH SETTINGS *****************************************************/
#ifdef USE_LFS_ROM
#   define LFS_ROM_LETTER           "F:"
#   define LFS_ROM_BLOCK_COUNT      64   /* 256k */              
#   define LFS_ROM_BLOCK_SIZE       4096 /* DO NOT EDIT - ERASE PAGE   */ 
#   define LFS_ROM_PROG_SIZE        256  /* DO NOT EDIT - WRITE SECTOR */
#   define LFS_ROM_READ_SIZE        256  
#   define LFS_ROM_CACHE_SIZE       256
#   define LFS_ROM_LOOKAHEAD_SIZE   64
#   define LFS_ROM_BLOCK_CYCLES     1000
//#   define LFS_ROM_PRE_FORMAT       /* ONLY FOR TEST */
#endif /* USE_LFS_ROM_FS ***********************************************/

#endif /* USE_LFS */

/* Enable FatFS */
#define USE_FATFS 
#ifdef USE_FATFS
// TODO
#endif /* USE_FATFS */

#endif