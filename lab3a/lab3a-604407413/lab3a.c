#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef unsigned short uut_16;
typedef unsigned long uut_32;

 int directory1;
 int indirect1;

 struct superblock{
   uut_16 magic_number;
   uut_32 totalnuminodes;
   uut_32 totalnumblocks;
   uut_32 blocksize;
   uut_32 fragsize;
   uut_32 blockspergroup;
   uut_32 inodespergroup;
   uut_32 fragspergroup;
   uut_32 firstdatablock;
   uut_32 firstinode;
   uut_32 fullinodes;
 };


struct superblock a = {0, 0, 0, 0, 0, 0, 0, 0, 0};


 struct group_descriptor{
   uut_16 freeblocks;
   uut_16 freeinodes;
   uut_16 numdirectories;
   uut_32 inodebitmap;
   uut_32 blockbitmap;
   uut_32 inodetable;
 };

struct group_descriptor * b;


 struct inode{
   uut_32 inodenum;
   uut_16 filemode;
   char filetype;
   uut_16 owner;
   uut_16 group;
   uut_16 linkcount;
   uut_32 create;
   uut_32 modification;
   uut_32 access;
   uut_32 filesize;
   uut_32 numblocks; //512 size of them
   uut_32 blockpointers[15];
 };

struct inode * inode_arr;

 struct directory{
   uut_32 entrynum;
   uut_32 entry_inode;
   uut_32 parent_inode;
   uut_16 rec_len;
   uut_16 name_len;
   char file_type;
   char name;
 };

struct directory d = {0, 0, 0, 0 ,0 ,0 ,0};

int fd;

void block_pointer_search(uut_32 i, uut_32 j, long long *entry_counter);

int main(int argc, char *argv[])
{

 
 char* file_system  = argv[1];
 fd = open(file_system, O_RDONLY);

 int csv1;
 csv1 = open("super.csv", O_WRONLY | O_CREAT | O_TRUNC, 0640);


 
 //super.csv
 pread(fd, &a.firstinode, 4, 1108);
 pread(fd, &a.magic_number, 2, 1080);
 pread(fd, &a.totalnuminodes, 4, 1024);
 pread(fd, &a.totalnumblocks, 4, 1028);
 pread(fd, &a.blocksize, 4, 1048); 
 a.blocksize = 1024 << a.blocksize;
 pread(fd, &a.fragsize, 4, 1052);
 a.fragsize = 1024 << a.fragsize;
 pread(fd, &a.blockspergroup, 4, 1056 );
 pread(fd, &a.inodespergroup, 4, 1064);
 pread(fd, &a.fragspergroup, 4, 1060);
 pread(fd, &a.firstdatablock, 4, 1044);

 dprintf(csv1, "%x,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\n", a.magic_number, a.totalnuminodes, a.totalnumblocks, a.blocksize, a.fragsize, a.blockspergroup, a.inodespergroup, a.fragspergroup, a.firstdatablock);
 
 close(csv1);

 

 
 //group.csv
 int group1;
 group1 = open("group.csv", O_WRONLY | O_CREAT | O_TRUNC, 0640);
 
 uut_32 numblockdescriptors;
 uut_32 modulo = a.totalnumblocks % a.blockspergroup;

 if(modulo == 0)
   numblockdescriptors = a.totalnumblocks / a.blockspergroup;
 else
   numblockdescriptors = (a.totalnumblocks / a.blockspergroup) + 1;

 
 a.fullinodes = numblockdescriptors * a.inodespergroup + numblockdescriptors;

 b = malloc(numblockdescriptors * sizeof(struct group_descriptor));

 for(int k = 0; k < numblockdescriptors; k++)
   {
     b[k].freeblocks = 0;
     b[k].freeinodes = 0;
     b[k].numdirectories = 0;
     b[k].inodebitmap = 0;
     b[k].blockbitmap = 0;
     b[k].inodetable = 0;
   }
 
 for(int i = 0; i < numblockdescriptors; i++)
   {
     if(i + 1 != numblockdescriptors)
       {  
	 if(a.blocksize == 1024)
	   {
	     pread(fd, &(b[i].freeblocks), 2, 12 + ((i*32) + 2048));
	     pread(fd, &(b[i].freeinodes), 2, 14 + ((i*32) + 2048));
	     a.fullinodes = a.fullinodes - b[i].freeinodes;
	     pread(fd, &(b[i].numdirectories), 2, 16 + ((i*32) + 2048));
	     pread(fd, &(b[i].inodebitmap), 4, 4 + ((i*32) + 2048));
	     pread(fd, &(b[i].blockbitmap), 4,((i*32) + 2048));
	     pread(fd, &(b[i].inodetable), 4,8 + ((i*32) + 2048));

	     dprintf(group1, "%lu,%u,%u,%u,%lx,%lx,%lx\n", a.blockspergroup, b[i].freeblocks, b[i].freeinodes, b[i].numdirectories, b[i].inodebitmap, b[i].blockbitmap, b[i].inodetable);
	   }

	 else
	   {
	     pread(fd, &(b[i].freeblocks), 2, 12 + ((i*32) + a.blocksize));
	     pread(fd, &(b[i].freeinodes), 2, 14 + ((i*32) + a.blocksize));
	     a.fullinodes = a.fullinodes - b[i].freeinodes;
	     pread(fd, &(b[i].numdirectories), 2, 16 + ((i*32) + a.blocksize));		     pread(fd, &(b[i].inodebitmap), 4, 4 + ((i*32) + a.blocksize));
	     pread(fd, &(b[i].blockbitmap), 4,((i*32) + a.blocksize));
	     pread(fd, &(b[i].inodetable), 4,8 + ((i*32) + a.blocksize));

	     dprintf(group1, "%lu,%u,%u,%u,%lx,%lx,%lx\n", a.blockspergroup, b[i].freeblocks, b[i].freeinodes, b[i].numdirectories, b[i].inodebitmap, b[i].blockbitmap, b[i].inodetable);

	   }
	 
       }

     else
       {

	 if(a.blocksize == 1024)
	   {
	     pread(fd, &b[i].freeblocks, 2, 12 + ((i*32) + 2048));
	     pread(fd, &b[i].freeinodes, 2, 14 + ((i*32) + 2048));
	     a.fullinodes = a.fullinodes - b[i].freeinodes;
	     pread(fd, &b[i].numdirectories, 2, 16 + ((i*32) + 2048));
	     pread(fd, &b[i].inodebitmap, 4, 4 + ((i*32) + 2048));
	     pread(fd, &b[i].blockbitmap, 4,((i*32) + 2048));
	     pread(fd, &b[i].inodetable, 4,8 + ((i*32) + 2048));

	     dprintf(group1, "%lu,%u,%u,%u,%lx,%lx,%lx\n", (a.totalnumblocks - (i*a.blockspergroup)) , b[i].freeblocks, b[i].freeinodes, b[i].numdirectories, b[i].inodebitmap, b[i].blockbitmap, b[i].inodetable);
	   }

	 else
	   {
	     
	     pread(fd, &b[i].freeblocks, 2, 12 + ((i*32) + a.blocksize));
	     pread(fd, &b[i].freeinodes, 2, 14 + ((i*32) + a.blocksize));
	     a.fullinodes = a.fullinodes - b[i].freeinodes;
	     pread(fd, &b[i].numdirectories, 2, 16 + ((i*32) + a.blocksize));
	     pread(fd, &b[i].inodebitmap, 4, 4 + ((i*32) + a.blocksize));
	     pread(fd, &b[i].blockbitmap, 4,((i*32) + a.blocksize));
	     pread(fd, &b[i].inodetable, 4,8 + ((i*32) + a.blocksize));

	     dprintf(group1, "%lu,%u,%u,%u,%lx,%lx,%lx\n", (a.totalnumblocks - (i*a.blockspergroup)), b[i].freeblocks, b[i].freeinodes, b[i].numdirectories, b[i].inodebitmap, b[i].blockbitmap, b[i].inodetable);

	   }
	 
       }
   }

 close(group1);
 
 //bitmap.csv

 int bitmap1;
 bitmap1 = open("bitmap.csv", O_WRONLY | O_TRUNC | O_CREAT, 0640);
 
 for(int j = 0; j < numblockdescriptors; j++)
   {
	 char byte;
	 short bit;
	 for(int i = 0; i <= a.blocksize; i++)
	   {
	     pread(fd, &byte, 1, (b[j].blockbitmap * a.blocksize) + i);
	     char mask = 1;
	     for(int kthbit = 1; kthbit <= 8; kthbit++)
	       {
		 if((i * 8) + kthbit <= a.blockspergroup)
		   {
		 
		     bit = byte & mask;
		     if(bit == 0)
		       {
			 dprintf(bitmap1, "%lx,", b[j].blockbitmap);
			 dprintf(bitmap1, "%lu", ((i * 8) + kthbit) + (j * a.blockspergroup));
			 dprintf(bitmap1, "%c", '\n');
		       }

		     mask = mask << 1;
		   }
	       }
	     
	   }
	 
	 for(int i = 0; i < a.blocksize; i++)
	   {
	     pread(fd, &byte, 1, (b[j].inodebitmap * a.blocksize) + i);
	     char mask = 1;
	     for(int kthbit = 1; kthbit <= 8; kthbit++)
	       {
		 if((i * 8) + kthbit <= a.inodespergroup)
		   {
		     bit = byte & mask;
		     if(bit == 0)
		       {
			 dprintf(bitmap1, "%lx,", b[j].inodebitmap);
			 dprintf(bitmap1, "%lu", ((i * 8) + kthbit) + (j * a.inodespergroup));
			 dprintf(bitmap1, "%c", '\n');	
		       }
		   }

	       mask = mask << 1;
	       }
	     
	   }
   }

 close(bitmap1);

 //inode.csv
 int inode1;
 inode1 = open("inode.csv", O_WRONLY | O_TRUNC | O_CREAT, 0640);

 inode_arr = malloc(a.fullinodes * sizeof(struct inode));
 
 for(int i = 0; i < a.fullinodes; i++)
   {
     inode_arr[i].inodenum = 0;
     inode_arr[i].filemode = 0;
     inode_arr[i].owner = 0;
     inode_arr[i].group = 0;
     inode_arr[i].filetype = '?';
     inode_arr[i].linkcount = 0;
     inode_arr[i].create = 0;
     inode_arr[i].modification = 0;
     inode_arr[i].access = 0;
     inode_arr[i].filesize = 0;
     inode_arr[i].numblocks = 0;
     for(int j = 0; j < 15; j++)
       {
	 inode_arr[i].blockpointers[j] = 0;
       }
	  
   }

 long long count = 0;
 
 for(int j = 0; j < numblockdescriptors; j++)
   {
	 char byte;
	 short bit;
	 
	 for(int i = 0; i < a.blocksize; i++)
	   {
	     pread(fd, &byte, 1, (b[j].inodebitmap * a.blocksize) + i);
	     char mask = 1;
	     for(int kthbit = 1; kthbit <= 8; kthbit++)
	       {
		 if((i * 8) + kthbit <= a.inodespergroup)
		   {
		     bit = byte & mask;
		     
		     if(bit != 0)
		       {
			
			 
			 inode_arr[count].inodenum = ((i * 8) + kthbit) + (j * a.inodespergroup);
			 dprintf(inode1, "%lu,", inode_arr[count].inodenum);

			 pread(fd, &inode_arr[count].filemode, 2, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))));

			 if(inode_arr[count].filemode >= 40960 && inode_arr[count].filemode <= 45055)
			   inode_arr[count].filetype = 's';
			 else if(inode_arr[count].filemode >= 32768 && inode_arr[count].filemode <= 36863)
			   inode_arr[count].filetype = 'f';
			 else if(inode_arr[count].filemode >= 16384 && inode_arr[count].filemode <= 20479)
			   inode_arr[count].filetype = 'd';
			 else
			   inode_arr[count].filetype = '?';

			 dprintf(inode1, "%c,", inode_arr[count].filetype);

			 dprintf(inode1, "%o,", inode_arr[count].filemode);
			   
			 pread(fd, &inode_arr[count].owner, 2, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 2);
			 dprintf(inode1,"%u,", inode_arr[count].owner);

			 pread(fd, &inode_arr[count].group, 2, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 24);
			 dprintf(inode1,"%u,", inode_arr[count].group);

			 pread(fd, &inode_arr[count].linkcount, 2, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 26);
			 dprintf(inode1, "%u,", inode_arr[count].linkcount);


			 
			 pread(fd, &inode_arr[count].create, 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 12);

			 dprintf(inode1, "%lx,", inode_arr[count].create);
			 
			 pread(fd, &inode_arr[count].modification, 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 16);

			 dprintf(inode1, "%lx,", inode_arr[count].modification);
			 
			 pread(fd, &inode_arr[count].access, 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 8);

			 dprintf(inode1, "%lx,", inode_arr[count].access);

			 pread(fd, &inode_arr[count].filesize, 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 4);
			 
			 dprintf(inode1, "%lu,", inode_arr[count].filesize);

			  pread(fd, &inode_arr[count].numblocks, 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 28);

			  dprintf(inode1, "%lu,", inode_arr[count].numblocks / (2 << a.blocksize) );

			 for(int u = 0; u < 14; u++)
			   {
			     pread(fd, &inode_arr[count].blockpointers[u], 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 40 + (4 * u));
			     dprintf(inode1, "%lx,", inode_arr[count].blockpointers[u]);
			   }

			 pread(fd, &inode_arr[count].blockpointers[14], 4, (b[j].inodetable * a.blocksize) + (128 * (inode_arr[count].inodenum- 1 - (a.inodespergroup * j))) + 40 + (4 * 14));
			 dprintf(inode1, "%lx\n", inode_arr[count].blockpointers[14]);
			 
			 
			count++; 
			 
			 
		       }
				 
		       mask = mask << 1;

		   }
	       }
	     
	   }
   }

 
 close(inode1);
   
   
 
 //directory.csv
 
 directory1 = open("directory.csv", O_WRONLY | O_TRUNC | O_CREAT, 0640);

 long long entry_counter;
 uut_32 dir_ptr;
 uut_32 dir_ptr2;
 uut_32 dir_ptr3;
 dir_ptr = 0;
 dir_ptr2 = 0;
 dir_ptr3 = 0;
 for(int i = 0; i < a.fullinodes; i++)
   {
     if(inode_arr[i].filetype == 'd')
       {
	 int j;
	 entry_counter = 0;
	 for(j = 0; j < 12; j++)
	   {
	     if(inode_arr[i].blockpointers[j] != 0)
	     {
	       
	       block_pointer_search(i,inode_arr[i].blockpointers[j], &entry_counter);
	     }
	   }
	 
	 j = 12;

	 if(inode_arr[i].blockpointers[j] != 0)
	   {
	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &dir_ptr, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		 if(dir_ptr != 0)
		   {
		     block_pointer_search(i, dir_ptr, &entry_counter);
		   }
	       }
	   }

	 j = 13;
	 dir_ptr = 0;
	 dir_ptr2 = 0;
	 if(inode_arr[i].blockpointers[j] != 0)
	   {
	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &dir_ptr2, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  if(dir_ptr2 != 0)
		   {
		     for(int w = 0; w < a.blocksize / 4; w++)
		       {
			 pread(fd, &dir_ptr, 4, dir_ptr2 * a.blocksize + (w * 4));
			 if(dir_ptr != 0)
			   {
			     block_pointer_search(i, dir_ptr, &entry_counter);
			   }
			 
		       }

		   }
		  
	       }
	   }
	 
	 dir_ptr = 0;
	 dir_ptr2 = 0;
	 dir_ptr3 = 0;
	 
	 j = 14;

	  if(inode_arr[i].blockpointers[j] != 0)
	   {
	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &dir_ptr3, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  if(dir_ptr3 != 0)
		   {
		     for(int w = 0; w < a.blocksize / 4; w++)
		       {
			 pread(fd, &dir_ptr2, 4, dir_ptr3 * a.blocksize + (w * 4));
			 if(dir_ptr2 != 0)
			   {
			     for(int x = 0; x < a.blocksize / 4; x++)
			       {
				 pread(fd, &dir_ptr, 4, dir_ptr2 * a.blocksize + (x * 4));
				 if(dir_ptr != 0)
				   {
				     block_pointer_search(i, dir_ptr, &entry_counter);
				   }
			       }
			 
		
			   }
			 
		       }
		     
		   }

	       }
	   }
       }
   }

 close(directory1);

 //indirect.csv
 indirect1 = open("indirect.csv", O_WRONLY | O_TRUNC | O_CREAT, 0640);

 uut_32 ptr;
 uut_32 ptr2;
 uut_32 ptr3;
 ptr = 0;
 ptr2 = 0;
 ptr3 = 0;
 for(int i = 0; i < a.fullinodes; i++)
   {
	 int j;
	 j = 12;
	 
	 int count = 0;
	 if(inode_arr[i].blockpointers[j] != 0)
	   {

	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &ptr, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		 if(ptr != 0)
		   {
		     dprintf(indirect1,"%lx,", inode_arr[i].blockpointers[j]);
		     dprintf(indirect1,"%u,%lx\n", count, ptr);
		     count++;
		   }
	       }
	   }

	 int count2;
	 count = 0;
	 count2 = 0;
	 j = 13;
	 ptr = 0;
	 ptr2 = 0;
	 if(inode_arr[i].blockpointers[j] != 0)
	   {

	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &ptr2, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  if(ptr2 != 0)
		   {
		     dprintf(indirect1,"%lx,", inode_arr[i].blockpointers[j]);
		     dprintf(indirect1,"%u,%lx\n", count2, ptr2);
		     count2++;
		   }
	       }
	   }

	       
	 count = 0;
	 count2 = 0;
	 j = 13;
	 ptr = 0;
	 ptr2 = 0;
	 if(inode_arr[i].blockpointers[j] != 0)
	   {

	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &ptr2, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  count = 0;
		 for(int w = 0; w < a.blocksize / 4; w++)
		   {
		     pread(fd, &ptr, 4, ptr2 * a.blocksize + (w * 4));
		     if(ptr != 0)
		       {
			 dprintf(indirect1,"%lx,", ptr2);
			 dprintf(indirect1,"%u,%lx\n", count, ptr);
			 count++;
		       }
		   
	           }
	       }
	   }
	 
	 int count3;
	 count = 0;
	 count2 = 0;
	 count3 = 0;
	 j = 14;
	 ptr = 0;
	 ptr2 = 0;
	 ptr3 = 0;
	 
	 if(inode_arr[i].blockpointers[j] != 0)
	   {

	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &ptr3, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  if(ptr3 != 0)
		   {
		     dprintf(indirect1, "%lx,", inode_arr[i].blockpointers[j]);
		     dprintf(indirect1, "%u,%lx\n", count3, ptr3);
		     count3++;
		   }
	
		 
	       }
	  }

	 
	 count = 0;
	 count2 = 0;
	 count3 = 0;
	 j = 14;
	 ptr = 0;
	 ptr2 = 0;
	 ptr3 = 0;
	 
	 if(inode_arr[i].blockpointers[j] != 0)
	   {

	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &ptr3, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  count2 = 0; 
		 for(int w = 0; w < a.blocksize / 4; w++)
		   {
		     pread(fd, &ptr2, 4, ptr3 * a.blocksize + (w * 4));
		     if(ptr2 != 0)
		       {
			 dprintf(indirect1, "%lx,", ptr3);
			 dprintf(indirect1, "%u,%lx\n", count2, ptr2);
			 count2++;
		       }

	           }
		}
	  }

	 count = 0;
	 count2 = 0;
	 count3 = 0;
	 j = 14;
	 ptr = 0;
	 ptr2 = 0;
	 ptr3 = 0;
	 
	 if(inode_arr[i].blockpointers[j] != 0)
	   {

	     for(int q = 0; q < a.blocksize / 4; q++)
	       {
		 pread(fd, &ptr3, 4, inode_arr[i].blockpointers[j] * a.blocksize + (q * 4));
		  count2 = 0; 
		 for(int w = 0; w < a.blocksize / 4; w++)
		   {
		     pread(fd, &ptr2, 4, ptr3 * a.blocksize + (w * 4));
		     count = 0;
		     for(int x = 0; x < a.blocksize / 4; x++)
		       {
			 pread(fd, &ptr, 4, ptr2 * a.blocksize + (x * 4));
			 if(ptr != 0)
			   {
			     dprintf(indirect1,"%lx,", ptr2);
			     dprintf(indirect1,"%u,%lx\n", count, ptr);
			     count++;
			   }
		       }
		   
	           }
		}
	  }
   }
  
 return 0;
 
}

//i is group descriptor j is block pointer number
void block_pointer_search(uut_32 i, uut_32 block, long long *entry_counter)
{
  
 long long entry_pointer;

 entry_pointer = 0;	     

     int valid = 1;
     while(entry_pointer < a.blocksize)
       {
	 d.rec_len = 0;
	 d.entry_inode = 0; 	 

	 pread(fd, &d.entry_inode, 4, (block * a.blocksize) + entry_pointer);   

	 if(d.entry_inode != 0 && valid == 1)

	   {			 		 			       
	     
	     dprintf(directory1 , "%lu,", inode_arr[i].inodenum);
	     dprintf(directory1 , "%llu,", *entry_counter);
	     
	     pread(fd , &d.rec_len, 2, ((block * a.blocksize) + entry_pointer + 4));
	     dprintf(directory1 , "%u,", d.rec_len);
	     
	     pread(fd, &d.name_len, 1, ((block * a.blocksize) + entry_pointer + 6));
	     dprintf(directory1 , "%u,",d.name_len);
	     dprintf(directory1 , "%lu,\"", d.entry_inode);
	     
	     char*ptr;
	     ptr = &d.name;
	     for(int l = 0; l < d.name_len; l++)
	       {
		 pread(fd, ptr, 1, ((block * a.blocksize) + entry_pointer + 8 + l));
		 dprintf(directory1, "%c", *ptr);
	       }
	     dprintf(directory1, "\"%c", '\n');
			 
	     entry_pointer = entry_pointer + d.rec_len;
	     *entry_counter = *entry_counter + 1;
	     
	   }
	 
	 
	 else if(d.entry_inode != 0 && valid == 0)
	   {
	     for(int k = 0; k < a.fullinodes; k++)
	       {
		 if(inode_arr[k].inodenum == d.entry_inode)
		   {
		     valid = 1;
		     *entry_counter = *entry_counter + 1;
		     break;
		   }
	       }
	     
	     if(valid == 0)
	       entry_pointer = entry_pointer + 4;
	   }
		     
	 else
	   {
	     valid = 0;
	     entry_pointer = entry_pointer + 4;
	   }
       } 
}
