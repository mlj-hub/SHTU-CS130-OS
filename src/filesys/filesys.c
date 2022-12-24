#include "filesys/filesys.h"
#include <debug.h>
#include <stdio.h>
#include <string.h>
#include "filesys/file.h"
#include "filesys/free-map.h"
#include "filesys/inode.h"
#include "filesys/directory.h"
#include "threads/thread.h"
#include "threads/malloc.h"

/* Partition that contains the file system. */
struct block *fs_device;

static void do_format (void);

/* Initializes the file system module.
   If FORMAT is true, reformats the file system. */
void
filesys_init (bool format) 
{
  fs_device = block_get_role (BLOCK_FILESYS);
  if (fs_device == NULL)
    PANIC ("No file system device found, can't initialize file system.");

  inode_init ();
  free_map_init ();

  if (format) 
    do_format ();

  free_map_open ();
}

/* Shuts down the file system module, writing any unwritten data
   to disk. */
void
filesys_done (void) 
{
  free_map_close ();
  cache_done();
}

/* Creates a file named NAME with the given INITIAL_SIZE.
   Returns true if successful, false otherwise.
   Fails if a file named NAME already exists,
   or if internal memory allocation fails. */
bool
filesys_create (const char *name, off_t initial_size,int is_dir) 
{
  block_sector_t inode_sector = 0;

  char * dir_path = calloc(1,strlen(name)+1);
  char * file_name = calloc(1,strlen(name)+1);
  path_split(name,dir_path,file_name);
  // printf("dir path:%s\n",dir_path);
  // printf("file name:%s\n",file_name);
  struct dir *dir = dir_open_path (dir_path);
  // if(dir == NULL)
  // {
  //   printf("null dir\n");
  // }
  
  bool success = (dir != NULL
                  && free_map_allocate (1, &inode_sector)
                  && inode_create (inode_sector, initial_size,is_dir)
                  && dir_add (dir, file_name, inode_sector,is_dir));
  // bool success = dir!=NULL;
  // printf("success dir:%d\n",success);
  // success = success&&free_map_allocate (1, &inode_sector);
  // printf("success free map:%d\n",success);
  // success = success&&inode_create (inode_sector, initial_size,is_dir);
  // printf("success inode:%d\n",success);
  // success = success&&dir_add (dir, file_name, inode_sector,is_dir);
  // printf("success add:%d\n",success);
  if (!success && inode_sector != 0) 
    free_map_release (inode_sector, 1);
  dir_close (dir);

  // if(success)
  // {
  //   printf("stop here\n");
  // }
  // else
  //   printf("fail create\n");

  return success;
}

/* Opens the file with the given NAME.
   Returns the new file if successful or a null pointer
   otherwise.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
struct file *
filesys_open (const char *name)
{
  if(strlen(name)<=0)
    return NULL;
  // int a = strlen(name);
  // printf("a:%d\n",a);
  char * dir_path = calloc(1,strlen(name)+1);
  char * file_name = calloc(1,strlen(name)+1);
  path_split(name,dir_path,file_name);
  struct dir *dir = dir_open_path (dir_path);
  struct inode *inode = NULL;

  if(dir == NULL)
  {
    free(dir_path);
    free(file_name);
    return NULL;
  }

  if(strlen(file_name)>0)
  {
    dir_lookup (dir, file_name, &inode);
    dir_close (dir);
  }
  else
    inode = dir_get_inode(dir);


  free(dir_path);
  free(file_name);
  return file_open (inode);
}

/* Deletes the file named NAME.
   Returns true if successful, false on failure.
   Fails if no file named NAME exists,
   or if an internal memory allocation fails. */
bool
filesys_remove (const char *name) 
{
  char * dir_path = calloc(1,strlen(name)+1);
  char * file_name = calloc(1,strlen(name)+1);
  path_split(name,dir_path,file_name);
  struct dir *dir = dir_open_path (dir_path);
  
  bool success = dir != NULL && dir_remove (dir, file_name);
  dir_close (dir); 
  free(dir_path);
  free(file_name);
  return success;
}

bool
filesys_cd(const char * name)
{
  struct dir * dir = dir_open_path(name);
  if(!dir)
    return false;
  struct thread * cur = thread_current();
  dir_close(cur->cwd);
  cur->cwd = dir;
  return true;
}


/* Formats the file system. */
static void
do_format (void)
{
  printf ("Formatting file system...");
  free_map_create ();
  if (!dir_create (ROOT_DIR_SECTOR, 16))
    PANIC ("root directory creation failed");
  free_map_close ();
  printf ("done.\n");
}
