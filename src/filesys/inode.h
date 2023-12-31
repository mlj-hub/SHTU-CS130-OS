#ifndef FILESYS_INODE_H
#define FILESYS_INODE_H

#include <stdbool.h>
#include "filesys/off_t.h"
#include "devices/block.h"
#include <list.h>


struct bitmap;

#define META_DATA_NUM 5
#define DIRECT_BLOCK_NUMBER ((BLOCK_SECTOR_SIZE-META_DATA_NUM*sizeof(block_sector_t))/sizeof(block_sector_t))
#define INDIRECT_BLOCK_NUMBER (BLOCK_SECTOR_SIZE/sizeof(block_sector_t))
#define DOUBLE_BLOCK_NUMBER (INDIRECT_BLOCK_NUMBER*INDIRECT_BLOCK_NUMBER)

#define DIRECT_INDEX_MAX 123
#define INDIRECT_INDEX_MAX (BLOCK_SECTOR_SIZE/sizeof(block_sector_t))
#define DOUBLE_INDIRECT_INDEX_MAX (BLOCK_SECTOR_SIZE*BLOCK_SECTOR_SIZE/sizeof(block_sector_t))

#define POINTER_PER_SECTOR (BLOCK_SECTOR_SIZE/sizeof(block_sector_t))

/* On-disk inode.
   Must be exactly BLOCK_SECTOR_SIZE bytes long. */
struct inode_disk
  {
    /* First data sector. */
    block_sector_t direct[DIRECT_BLOCK_NUMBER];
    /* Indirect pointer */
    block_sector_t indirect;               
    /* Double indirect pointer */
    block_sector_t double_indirect;
    /* File size in bytes. */
    off_t length;
    /* Is directory */           
    int32_t is_dir;
    /* Magic number. */
    unsigned magic;                     
  };

/* In-memory inode. */
struct inode 
  {
    struct list_elem elem;              /* Element in inode list. */
    block_sector_t sector;              /* Sector number of disk location. */
    int open_cnt;                       /* Number of openers. */
    bool removed;                       /* True if deleted, false otherwise. */
    int deny_write_cnt;                 /* 0: writes ok, >0: deny writes. */
    struct inode_disk data;             /* Inode content. */
  };

void inode_init (void);
bool inode_create (block_sector_t, off_t,int);
struct inode *inode_open (block_sector_t);
struct inode *inode_reopen (struct inode *);
block_sector_t inode_get_inumber (const struct inode *);
void inode_close (struct inode *);
void inode_remove (struct inode *);
off_t inode_read_at (struct inode *, void *, off_t size, off_t offset);
off_t inode_write_at (struct inode *, const void *, off_t size, off_t offset);
void inode_deny_write (struct inode *);
void inode_allow_write (struct inode *);
off_t inode_length (const struct inode *);

#endif /* filesys/inode.h */
