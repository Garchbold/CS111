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
#include <string.h>

FILE* fp;

//http://stackoverflow.com/questions/12911299/read-csv-file-in-c

const char* getfield(char* line, int num)
{
  const char* tok;
  for (tok = strtok(line, ",");
       tok && *tok;
       tok = strtok(NULL, ",\n"))
    {
      if (!--num)
	return tok;
    }
  return NULL;
}

int main(int argc, char *argv[])
{

  int file = open("lab3b_check.txt", O_WRONLY | O_CREAT | O_TRUNC, 0640);
  
  char line[2048];
  
  FILE* super = fopen("super.csv", "r");
  FILE* group = fopen("group.csv", "r");
  FILE* bitmap = fopen("bitmap.csv", "r");
  FILE* inode = fopen("inode.csv", "r");
  FILE* directory = fopen("directory.csv", "r");
  FILE* indirect = fopen("indirect.csv", "r");

  int inode_lines = 0;
  char ch;
  fp = fopen("inode.csv", "r");
  while((ch=fgetc(fp))!=EOF)
    {
      if (ch=='\n') {inode_lines++; }
    }
  fclose(fp);

  

  int bitmap_lines = 0;
  fp = fopen("bitmap.csv", "r");
  while((ch=fgetc(fp))!=EOF)
    {
      if (ch=='\n') {bitmap_lines++; }
    }
  fclose(fp);


  int group_lines = 0;
  
  fp=fopen("group.csv","r");
  while((ch=fgetc(fp))!=EOF)
    {
      if (ch=='\n') { group_lines++; }
    }
  fclose(fp);

  int directory_lines = 0;
  fp=fopen("directory.csv","r");
  
  int numquotations = 2;
  while(fgets(line, 2048, fp))
    {
      if(numquotations % 2 == 0)
	{
	  directory_lines++;
	  numquotations = 2;
	  for(int i = 0; i < 2048; i++)
	    {	      
	      if(line[i] == '\0')
		break;
	      
	      if(line[i] == '\"')
		numquotations++;
	    }	      
	}
      
      else
	{
	  for(int i = 0; i < 2048; i++)
	    {	      
	      if(line[i] == '\0')
		break;
	      
	      if(line[i] == '\"')
		numquotations++;
	    }
	}
    }


  fclose(fp);
    
  int indirect_lines = 0;
  fp=fopen("indirect.csv","r");
  while((ch=fgetc(fp))!=EOF)
    {
      if (ch=='\n') {indirect_lines++; }
    }
  fclose(fp);

  int inodespergroup = 0;
  int blockspergroup = 0;
  int blocksize = 0;
  int totalnumblocks = 0;
  while(fgets(line, 2048, super))
    {
      char* tmp = strdup(line);
      char* tmp1 = strdup(line);
      char* tmp2 = strdup(line);
      char* tmp3 = strdup(line);
      inodespergroup = atoi(getfield(tmp, 7));
      blockspergroup = atoi(getfield(tmp1, 6));
      blocksize = atoi(getfield(tmp2, 4));
      totalnumblocks = atoi(getfield(tmp3, 3));
    }
  
  
  struct inode{
    int inode_block_pointer[15];
    int link_count;
    int inode_num;
  };

  struct inode * list_inodes = malloc(sizeof(struct inode) * inode_lines);

  struct group{
    int group_block_num;
    int inode_block_num;
  };

  struct group * list_groups = malloc(sizeof(struct group) * group_lines);

  struct directory{
    int period_dir_current;
    int period_dir_parent;
    int dir_current;
    const char * dir_name;
    int entry;
    int dir_parent;
  };

  struct directory * list_directories = malloc(sizeof(struct directory) * directory_lines);
  
  struct bitmap{
    int group_num;
    int block_num;
  };

  struct bitmap * list_bitmaps = malloc(sizeof(struct bitmap) * bitmap_lines);
  

  struct indirect{
    int containing_block_num;
    int entry;
    int block_ptr;
  };

  struct indirect * list_indirects = malloc(sizeof(struct indirect) * indirect_lines);

  char line2[2048];

  
  //------group--------//
  int index = 0;
  while(fgets(line, 2048, group))
    {
      char* tmp = strdup(line);
      char* tmp1 = strdup(line);

      list_groups[index].inode_block_num=atoi(getfield(tmp, 5));
      list_groups[index].group_block_num=atoi(getfield(tmp1, 6));

      index++;
    }
  
  
  //------inode---------//
  index = 0;
  while(fgets(line, 2048, inode))
    {
      char* tmp = strdup(line);
      char* tmp1 = strdup(line);

      
      list_inodes[index].inode_num = atoi(getfield(tmp, 1));
      list_inodes[index].link_count = atoi(getfield(tmp1, 6));

      for(int i = 0; i < 15; i++)
	{
	  char* tmp2 = strdup(line);
	  const char *hexstring = getfield(tmp2, 12 + i);
	  list_inodes[index].inode_block_pointer[i] = (int) strtol(hexstring, NULL, 16);
	}

      index++;
    }

  //------directory------//

  index = 0;
  numquotations = 2;
  int period_counter = 0;
  while(fgets(line, 2048, directory))
  {
    if(numquotations % 2 == 0)
      {
	numquotations = 2;
	for(int j = 0; j < 2048; j++)
	  {
	    if(line[j] == '\0')
	      break;

	    if(line[j] == '\"')
	      numquotations++;
	  }

	//do stuff
	char* tmp = strdup(line);
	char* tmp1 = strdup(line);
	char* tmp2 = strdup(line);
	char* tmp3 = strdup(line);
	
	int curr_inode = atoi(getfield(tmp,1));
	int parent_inode = atoi(getfield(tmp2,5));
	int entry = atoi(getfield(tmp1, 2));
	
	list_directories[index].dir_current = curr_inode;
	list_directories[index].dir_parent = parent_inode;
	list_directories[index].entry = entry;

	const char* name = getfield(tmp3, 6);
	list_directories[index].dir_name = name;

	if(!strcmp(name, "\".\"") || !strcmp(name, "\"..\"")){
	    
	    list_directories[period_counter].period_dir_current = curr_inode;
	    list_directories[period_counter].period_dir_parent = parent_inode;
	    period_counter++;
	  }
	  index++;
      }
    else
      {
	for(int j = 0; j < 2048; j++)
	  {
	    if(line[j] == '\0')
	      break;
	    
	    if(line[j] == '\"')
	      numquotations++;
	  }
      }
  }
  
  index = 0;
  while(fgets(line, 2048, indirect))
    {
      char* tmp = strdup(line);
      char* tmp1= strdup(line);
      char* tmp2= strdup(line);
      
      list_indirects[index].containing_block_num=atoi(getfield(tmp, 1));
      list_indirects[index].entry = atoi(getfield(tmp1, 2));
      list_indirects[index].block_ptr = atoi(getfield(tmp2, 3));
      index++;
    }

  
  index = 0;
  while(fgets(line, 2048, bitmap))
    {
      char* tmp= strdup(line);
      char* tmp1= strdup(line);
            
      list_bitmaps[index].group_num = atoi(getfield(tmp, 1));
      list_bitmaps[index].block_num = atoi(getfield(tmp1, 2));
      
      index++;
      }


  //find all the inode entries that are not recorded free in the bitmap, but are
  for(int i = 0; i < inode_lines; i++)
    {
      int found = 0;
      for(int j = 0; j < directory_lines; j++)
	{
	  if(list_inodes[i].inode_num < 11 || list_inodes[i].inode_num == list_directories[j].dir_parent)
	    {
	      found = 1;
	      break;
	    }
	}

      if(found == 0)
	{
	  for(int k = 0; k < group_lines; k++)
	    {
	      if(list_inodes[i].inode_num >= (k * inodespergroup + 1) && list_inodes[i].inode_num <= inodespergroup * (k + 1))
		dprintf(file, "MISSING INODE < %d > SHOULD BE IN FREE LIST < %d >\n", list_inodes[i].inode_num, list_groups[k].inode_block_num);
	    }

	}

    }

  int inode_found;
  for(int j = 0; j < directory_lines; j++)
    {
      inode_found = 0;
      for(int i = 0; i < inode_lines; i++)
	{
	  if(list_directories[j].dir_parent == list_inodes[i].inode_num)
	    {
	      inode_found = 1;
	      break;
	    }
	}
      
      if(inode_found == 0)
	dprintf(file,"UNALLOCATED INODE < %d > REFERENCED BY DIRECTORY < %d > ENTRY < %d >\n", list_directories[j].dir_parent, list_directories[j].dir_current, list_directories[j].entry);   
    }


  int currentsize = 2048;
  int* multiple_inodes = (int *) malloc(currentsize * sizeof(int));
  int currentindex = 0;
  
  for(int i = 0; i < currentsize; i++)
      multiple_inodes[i] = -1;
    
  int block;
  for(int a = 0; a < inode_lines; a++)
    {
      for(int i = 0; i < 15; i++)
	{
	  int first_occurence = 1;
	  int newline = 1;
	  int traversed_block_found = 0;

	  if(i == 12)
	    {
	      if(list_inodes[a].inode_block_pointer[12] != 0)
		{
		  for(int b = 0; b < indirect_lines; b++)
		    {
		      if(list_inodes[a].inode_block_pointer[12] == list_indirects[b].containing_block_num)
			{
			  block = list_indirects[b].block_ptr;
			  if(block == 0 || block > totalnumblocks)
			    break;
			  for(int k = 0; k < inode_lines; k ++)
			    {  
			      if(block == multiple_inodes[k])
				{
				  traversed_block_found = 1;
				}
			    }
			  
			  if(!traversed_block_found)
			    {
			      for(int e = a + 1; a + 1 < inode_lines && e < inode_lines; e++)
				{
				  for(int j = 0; j < 15; j++)
				    {
				      if(j == 12)
					{
					  if(list_inodes[a].inode_block_pointer[12] != 0)
					    {
					      for(int b = 0; b < indirect_lines; b++)
						{
						  if(list_inodes[a].inode_block_pointer[12] == list_indirects[b].containing_block_num)
						    {
						      if(block == list_indirects[b].block_ptr)
							{
							  multiple_inodes[currentindex] = block;
							  
							  if(currentindex + 1 == currentsize)
							    {
							      currentsize *= 2;
							      multiple_inodes = realloc(multiple_inodes, currentsize);
							      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
								multiple_inodes[arr_ind] = -1;
							    }
							  
							  currentindex++;
							  
						  
							  if(first_occurence == 1)
							    {
							      first_occurence = 0;
							      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
							    }
						  
							  else
							    {
							      newline = 0;
							      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
							    }
							  
							}
						    }
						}
					    } 
					}
				      
				      else if(j == 13)
					{
					  if(list_inodes[a].inode_block_pointer[12] != 0)
					    {
					      for(int b = 0; b < indirect_lines; b++)
						{
						  if(list_inodes[a].inode_block_pointer[13] == list_indirects[b].containing_block_num)
						    {
						      for(int c = 0; c < indirect_lines; c++)
							{
							  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
							    {
							      if(block == list_indirects[c].block_ptr)
								{
								  multiple_inodes[currentindex] = block;
								  
								  if(currentindex + 1 == currentsize)
								    {
								      currentsize *= 2;
								      multiple_inodes = realloc(multiple_inodes, currentsize);
								      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
									multiple_inodes[arr_ind] = -1;
								    }
								  
								  currentindex++;
								  
								  
								  if(first_occurence == 1)
								    {
								      first_occurence = 0;
								      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
								    }
						       
								  else
								    {
								      newline = 0;
								      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
								    }
								  
			       
								}
							    }
							}
						    }
						}
					    }
					}
				      
				      else if(j == 14)
					{
					  if(list_inodes[a].inode_block_pointer[14] != 0)
					    {
					      for(int b = 0; b < indirect_lines; b++)
						{
						  if(list_inodes[a].inode_block_pointer[14] == list_indirects[b].containing_block_num)
						    {
						      for(int c = 0; c < indirect_lines; c++)
							{
							  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
							    {
							      for(int d = 0; d < indirect_lines; d++)
								{
								  if(list_indirects[c].block_ptr == list_indirects[d].containing_block_num)
								    {
								      if(block == list_indirects[d].block_ptr)
									{
									  multiple_inodes[currentindex] = block;
									  
									  if(currentindex + 1 == currentsize)
									    {
									      currentsize *= 2;
									      multiple_inodes = realloc(multiple_inodes, currentsize);
									      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
										multiple_inodes[arr_ind] = -1;
									    }
									  
									  currentindex++;
								  
									  
									  if(first_occurence == 1)
									    {
									      first_occurence = 0;
									      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
									    }
									  
									  else
									    {
									      newline = 0;
									      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
									    }
									  
									  
									}
								    }
							   
								}
							    }
							}
						    }
						}
					    }
					}
			    		  
				      
				      else
					{
					  if(block == list_inodes[e].inode_block_pointer[j])
					    {
					      multiple_inodes[currentindex] = block;
					      
					      if(currentindex + 1 == currentsize)
						{
						  currentsize *= 2;
						  multiple_inodes = realloc(multiple_inodes, currentsize);
						  for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
						    multiple_inodes[arr_ind] = -1;
						}
					      
					      currentindex++;
					      
					      
					      if(first_occurence == 1)
						{
						  first_occurence = 0;
						  dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
						}
					      
					      else
						{
						  newline = 0;
						  dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
						}
					    }
					}
				    }
				}
			    }
			}
		    }
		}
	      
	    }
       
	  else if(i == 13)
	    {
	      if(list_inodes[a].inode_block_pointer[12] != 0)
		{
		  for(int b = 0; b < indirect_lines; b++)
		    {
		      if(list_inodes[a].inode_block_pointer[13] == list_indirects[b].containing_block_num)
			{
			  for(int c = 0; c < indirect_lines; c++)
			    {
			      if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
				{
				  block = list_indirects[c].block_ptr;
				  if(block == 0 || block > totalnumblocks)
				    break;
				  for(int k = 0; k < inode_lines; k ++)
				    {  
				      if(block == multiple_inodes[k])
					{
					  traversed_block_found = 1;
					}
				    }

				  if(!traversed_block_found)
				    {
				      for(int e = a + 1; a + 1 < inode_lines && e < inode_lines; e++)
					{
					  for(int j = 0; j < 15; j++)
					    {
					      if(j == 12)
						{
						  if(list_inodes[a].inode_block_pointer[12] != 0)
						    {
						      for(int b = 0; b < indirect_lines; b++)
							{
							  if(list_inodes[a].inode_block_pointer[12] == list_indirects[b].containing_block_num)
							    {
							      if(block == list_indirects[b].block_ptr)
								{
								  multiple_inodes[currentindex] = block;
								  
								  if(currentindex + 1 == currentsize)
								    {
								      currentsize *= 2;
								      multiple_inodes = realloc(multiple_inodes, currentsize);
								      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
									multiple_inodes[arr_ind] = -1;
								    }
								  
								  currentindex++;
								  
								  
								  if(first_occurence == 1)
								    {
								      first_occurence = 0;
								      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
								    }
								  
								  else
								    {
								      newline = 0;
								      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
								    }
								  
								}
							    }
							}
						    } 
						}
					      
					      else if(j == 13)
						{
						  if(list_inodes[a].inode_block_pointer[12] != 0)
						    {
						      for(int b = 0; b < indirect_lines; b++)
							{
							  if(list_inodes[a].inode_block_pointer[13] == list_indirects[b].containing_block_num)
							    {
							      for(int c = 0; c < indirect_lines; c++)
								{
								  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
								    {
								      if(block == list_indirects[c].block_ptr)
									{
									  multiple_inodes[currentindex] = block;
									  
									  if(currentindex + 1 == currentsize)
									    {
									      currentsize *= 2;
									      multiple_inodes = realloc(multiple_inodes, currentsize);
									      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
										multiple_inodes[arr_ind] = -1;
									    }
									  
									  currentindex++;
									  
									  
									  if(first_occurence == 1)
									    {
									      first_occurence = 0;
									      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
									    }
						       
									  else
									    {
									      newline = 0;
									      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
									    }
									  
									  
									}
								    }
								}
							    }
							}
						    }
						}
					      
					      else if(j == 14)
						{
						  if(list_inodes[a].inode_block_pointer[14] != 0)
						    {
						      for(int b = 0; b < indirect_lines; b++)
							{
							  if(list_inodes[a].inode_block_pointer[14] == list_indirects[b].containing_block_num)
							    {
							      for(int c = 0; c < indirect_lines; c++)
								{
								  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
								    {
								      for(int d = 0; d < indirect_lines; d++)
									{
									  if(list_indirects[c].block_ptr == list_indirects[d].containing_block_num)
									    {
									      if(block == list_indirects[d].block_ptr)
										{
										  multiple_inodes[currentindex] = block;
										  
										  if(currentindex + 1 == currentsize)
										    {
										      currentsize *= 2;
										      multiple_inodes = realloc(multiple_inodes, currentsize);
										      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
											multiple_inodes[arr_ind] = -1;
										    }
								  
										  currentindex++;
										  
										  
										  if(first_occurence == 1)
										    {
										      first_occurence = 0;
										      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
										    }
										  
										  else
										    {
										      newline = 0;
										      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
										    }
										  
						   
										}
									    }
							   
									}
								    }
								}
							    }
							}
						    }
						}
					      
					      
					      else
						{
						  if(block == list_inodes[e].inode_block_pointer[j])
						    {
						      multiple_inodes[currentindex] = block;
						      
						      if(currentindex + 1 == currentsize)
							{
							  currentsize *= 2;
							  multiple_inodes = realloc(multiple_inodes, currentsize);
							  for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
							    multiple_inodes[arr_ind] = -1;
							}
						      
						      currentindex++;
						      
				      
						      if(first_occurence == 1)
							{
							  first_occurence = 0;
							  dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
							}
						      
						      else
							{
							  newline = 0;
					  dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
							}
						    }
						}
					    }
					}
				    }
				}
			    }
		        }
		    }
		}
	    }

	  else if(i == 14)
	    {
	      if(list_inodes[a].inode_block_pointer[14] != 0)
		{
		  for(int b = 0; b < indirect_lines; b++)
		    {
		      if(list_inodes[a].inode_block_pointer[14] == list_indirects[b].containing_block_num)
			{
			  for(int c = 0; c < indirect_lines; c++)
			    {
			      if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
				{
				  for(int d = 0; d < indirect_lines; d++)
				    {
				      if(list_indirects[c].block_ptr == list_indirects[d].containing_block_num)
					{
					  block = list_indirects[d].block_ptr;
					  if(block == 0 || block > totalnumblocks)
					    break;
					  for(int k = 0; k < inode_lines; k ++)
					    {  
					      if(block == multiple_inodes[k])
						{
						  traversed_block_found = 1;
						}
					    }

					  if(!traversed_block_found)
					    {
					      for(int e = a + 1; a + 1 < inode_lines && e < inode_lines; e++)
						{
						  for(int j = 0; j < 15; j++)
						    {
						      if(j == 12)
							{
							  if(list_inodes[a].inode_block_pointer[12] != 0)
							    {
							      for(int b = 0; b < indirect_lines; b++)
								{
								  if(list_inodes[a].inode_block_pointer[12] == list_indirects[b].containing_block_num)
								    {
								      if(block == list_indirects[b].block_ptr)
									{
									  multiple_inodes[currentindex] = block;
									  
									  if(currentindex + 1 == currentsize)
									    {
									      currentsize *= 2;
									      multiple_inodes = realloc(multiple_inodes, currentsize);
									      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
										multiple_inodes[arr_ind] = -1;
									    }
									  
									  currentindex++;
									  
									  
									  if(first_occurence == 1)
									    {
									      first_occurence = 0;
									      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
									    }
						  
									  else
									    {
									      newline = 0;
									      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
									    }
									  
									}
								    }
								}
							    } 
							}
						      
						      else if(j == 13)
							{
							  if(list_inodes[a].inode_block_pointer[12] != 0)
							    {
							      for(int b = 0; b < indirect_lines; b++)
								{
								  if(list_inodes[a].inode_block_pointer[13] == list_indirects[b].containing_block_num)
								    {
								      for(int c = 0; c < indirect_lines; c++)
									{
									  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
									    {
									      if(block == list_indirects[c].block_ptr)
										{
										  multiple_inodes[currentindex] = block;
							  
										  if(currentindex + 1 == currentsize)
										    {
										      currentsize *= 2;
										      multiple_inodes = realloc(multiple_inodes, currentsize);
										      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
											multiple_inodes[arr_ind] = -1;
										    }
										  
										  currentindex++;
										  
										  
										  if(first_occurence == 1)
										    {
										      first_occurence = 0;
										      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
										    }
										  
										  else
										    {
										      newline = 0;
										      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
										    }
										  
										  
										}
									    }
									}
								    }
								}
							    }
							}
						      
						      else if(j == 14)
							{
							  if(list_inodes[a].inode_block_pointer[14] != 0)
							    {
							      for(int b = 0; b < indirect_lines; b++)
								{
								  if(list_inodes[a].inode_block_pointer[14] == list_indirects[b].containing_block_num)
								    {
								      for(int c = 0; c < indirect_lines; c++)
									{
									  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
									    {
									      for(int d = 0; d < indirect_lines; d++)
										{
										  if(list_indirects[c].block_ptr == list_indirects[d].containing_block_num)
										    {
										      if(block == list_indirects[d].block_ptr)
											{
											  multiple_inodes[currentindex] = block;
											  
											  if(currentindex + 1 == currentsize)
											    {
											      currentsize *= 2;
											      multiple_inodes = realloc(multiple_inodes, currentsize);
											      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
												multiple_inodes[arr_ind] = -1;
											    }
											  
											  currentindex++;
								  
											  
											  if(first_occurence == 1)
											    {
											      first_occurence = 0;
											      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
											    }
											  
											  else
											    {
											      newline = 0;
											      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
											    }
											  
											  
											}
										    }
							   
										}
									    }
									}
								    }
								}
							    }
							}
			    		  
						      
						      else
							{
							  if(block == list_inodes[e].inode_block_pointer[j])
							    {
							      multiple_inodes[currentindex] = block;
							      
							      if(currentindex + 1 == currentsize)
								{
								  currentsize *= 2;
								  multiple_inodes = realloc(multiple_inodes, currentsize);
								  for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
								    multiple_inodes[arr_ind] = -1;
								}
							      
							      currentindex++;
							      
							      
							      if(first_occurence == 1)
								{
								  first_occurence = 0;
								  dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
								}
				  
							      else
								{
								  newline = 0;
								  dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
								}
							    }
							}
						    }
						}
					    }
					  
					}
				    }
				}
			    }
			}
		    }
		}
	    }		  
	  
	  else
	    {
	      block = list_inodes[a].inode_block_pointer[i];
	      if(list_inodes[a].inode_block_pointer[i] != 0)
		{
		  for(int k = 0; k < inode_lines; k ++)
		    {
		      if(block > totalnumblocks)
			break;
		      if(block == multiple_inodes[k])
			{
			  traversed_block_found = 1;
			}	      
		    }

		  if(!traversed_block_found)
		    {
		      for(int e = a + 1; a + 1 < inode_lines && e < inode_lines; e++)
			{
			  for(int j = 0; j < 15; j++)
			    {
			      if(j == 12)
				{
				  if(list_inodes[a].inode_block_pointer[12] != 0)
				    {
				      for(int b = 0; b < indirect_lines; b++)
					{
					  if(list_inodes[a].inode_block_pointer[12] == list_indirects[b].containing_block_num)
					    {
					      if(block == list_indirects[b].block_ptr)
						{
					       	  multiple_inodes[currentindex] = block;
						  
						  if(currentindex + 1 == currentsize)
						    {
						      currentsize *= 2;
						      multiple_inodes = realloc(multiple_inodes, currentsize);
						      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
							multiple_inodes[arr_ind] = -1;
						    }
				  
						  currentindex++;
						  
						  
						  if(first_occurence == 1)
						    {
						      first_occurence = 0;
						      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
						    }
						  
						  else
						    {
						      newline = 0;
						      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
						    }

					     }
					 }
				     }
		        	   } 
				}
       
			      else if(j == 13)
				{
				  if(list_inodes[a].inode_block_pointer[12] != 0)
				    {
				      for(int b = 0; b < indirect_lines; b++)
					{
					  if(list_inodes[a].inode_block_pointer[13] == list_indirects[b].containing_block_num)
					    {
					      for(int c = 0; c < indirect_lines; c++)
						{
						  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
						    {
						      if(block == list_indirects[c].block_ptr)
							{
							  multiple_inodes[currentindex] = block;
							  
							  if(currentindex + 1 == currentsize)
							    {
							      currentsize *= 2;
							      multiple_inodes = realloc(multiple_inodes, currentsize);
							      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
							     multiple_inodes[arr_ind] = -1;
							    }
							  
							  currentindex++;
						       
						  
							  if(first_occurence == 1)
							    {
							      first_occurence = 0;
							      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
							    }
						       
							  else
							    {
							      newline = 0;
							      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
							    }
							  
			       
							}
						    }
						}
					    }
					}
				    }
				}
			   
			      else if(j == 14)
				{
				  if(list_inodes[a].inode_block_pointer[14] != 0)
				    {
				      for(int b = 0; b < indirect_lines; b++)
					{
					  if(list_inodes[a].inode_block_pointer[14] == list_indirects[b].containing_block_num)
					    {
					      for(int c = 0; c < indirect_lines; c++)
						{
						  if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
						    {
						      for(int d = 0; d < indirect_lines; d++)
							{
							  if(list_indirects[c].block_ptr == list_indirects[d].containing_block_num)
							    {
							      if(block == list_indirects[d].block_ptr)
								{
								  multiple_inodes[currentindex] = block;
								  
								  if(currentindex + 1 == currentsize)
								    {
								      currentsize *= 2;
								      multiple_inodes = realloc(multiple_inodes, currentsize);
								      for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
									multiple_inodes[arr_ind] = -1;
								    }
								  
								  currentindex++;
								  
								  
								  if(first_occurence == 1)
								    {
								      first_occurence = 0;
								      dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
								    }
								  
								  else
								    {
								      newline = 0;
								      dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
								    }
								  
						   
								}
							     }
							   
							 }
						     }
						 }
					    }
					}
				    }
				}
			    		  
			  
			      else
				{
				  if(block == list_inodes[e].inode_block_pointer[j])
				    {
				      multiple_inodes[currentindex] = block;
				      
				      if(currentindex + 1 == currentsize)
					{
					  currentsize *= 2;
					  multiple_inodes = realloc(multiple_inodes, currentsize);
					  for(int arr_ind = currentindex + 1; arr_ind < currentsize; arr_ind++)
					    multiple_inodes[arr_ind] = -1;
					}
				      
				      currentindex++;
				      
				      
				      if(first_occurence == 1)
					{
					  first_occurence = 0;
					  dprintf(file,"MULTIPLY REFERENCED BLOCK < %d > BY INODE < %d > ENTRY < %d > INODE < %d > ENTRY < %d >", block, list_inodes[a].inode_num, i, list_inodes[e].inode_num, j);
					}
				  
				      else
					{
					  newline = 0;
					  dprintf(file," INODE < %d > ENTRY < %d >\n", list_inodes[e].inode_num, j);
					}
				    }
				}
			    }
			}
		    }
		  
		}
	    }
	  
	    
	      if(newline && !first_occurence)
		dprintf(file,"\n");
	}
    }
		      
	     	  

  
    for(int a = 0; a < inode_lines; a++)
      {
      for(int i = 0; i < 15; i++)
	{
	  if(i == 12)
	    {
	      if(list_inodes[a].inode_block_pointer[12] != 0)
		{
		  for(int b = 0; b < indirect_lines; b++)
		    {
		      if(list_inodes[a].inode_block_pointer[12] == list_indirects[b].containing_block_num)
			{

			  if(list_indirects[b].block_ptr == 0 || list_indirects[b].block_ptr > totalnumblocks)
			    {
			      dprintf(file,"INVALID BLOCK < %d > IN INODE < %d > INDIRECT BLOCK < %d > ENTRY < %d >", list_indirects[b].block_ptr, list_inodes[a].inode_num, list_inodes[a].inode_block_pointer[12], list_indirects[b].entry);
			    }
			  
			  for(int j = 0; j < bitmap_lines; j++)
			    {
			      if(list_bitmaps[j].block_num ==  list_indirects[b].block_ptr)
				{
				  for(int k = 0; k < group_lines; k++)
				    {
				      if(list_bitmaps[j].group_num == list_groups[k].group_block_num)
					{
					  dprintf(file,"INODE < %d > INDIRECT BLOCK < %d > ENTRY < %d >\n", list_inodes[a].inode_num, list_inodes[a].inode_block_pointer[12], list_indirects[b].entry);
					}
				    }
				}

			    }
			}
		    }
		}
	    }
	  
	  else if(i == 13)
	    {
	      if(list_inodes[a].inode_block_pointer[13] != 0)
		{
		  for(int b = 0; b < indirect_lines; b++)
		    {
		      if(list_inodes[a].inode_block_pointer[13] == list_indirects[b].containing_block_num)
			{
			  for(int c = 0; c < indirect_lines; c++)
			    {
			      if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
				{

				  if(list_indirects[c].block_ptr == 0 || list_indirects[c].block_ptr > totalnumblocks)
					    {
					      dprintf(file,"INVALID BLOCK < %d > IN INODE < %d > INDIRECT BLOCK < %d > ENTRY < %d >", list_indirects[c].block_ptr, list_inodes[a].inode_num, list_inodes[a].inode_block_pointer[13], list_indirects[c].entry + (blocksize/4 )* list_indirects[b].entry);
					    }

				  for(int j = 0; j < bitmap_lines; j++)
				    {
				      if(list_bitmaps[j].block_num ==  list_indirects[c].block_ptr)
					{
					  for(int k = 0; k < group_lines; k++)
					    {
					      if(list_bitmaps[j].group_num == list_groups[k].group_block_num)
						{
						  dprintf(file,"INODE < %d > INDIRECT BLOCK < %d > ENTRY < %d >\n", list_inodes[a].inode_num, list_inodes[a].inode_block_pointer[13], list_indirects[c].entry + (blocksize/4 )* list_indirects[b].entry); 
						}
					    }
					}
				      
				    }
				}
			    }
			}
		    }
		}
	    }

	  
	  
	  else if(i == 14)
	    {
	      if(list_inodes[a].inode_block_pointer[14] != 0)
		{
		  for(int b = 0; b < indirect_lines; b++)
		    {
		      if(list_inodes[a].inode_block_pointer[14] == list_indirects[b].containing_block_num)
			{
			  for(int c = 0; c < indirect_lines; c++)
			    {
			      if(list_indirects[b].block_ptr == list_indirects[c].containing_block_num)
				{
				  for(int d = 0; d < indirect_lines; d++)
				    {
				      if(list_indirects[c].block_ptr == list_indirects[d].containing_block_num)
					{

					  if(list_indirects[d].block_ptr == 0 || list_inodes[a].inode_block_pointer[i] > totalnumblocks)
					    {
					      dprintf(file,"INVALID BLOCK < %d > IN INODE < %d > INDIRECT BLOCK < %d > ENTRY < %d >", list_indirects[d].block_ptr, list_inodes[a].inode_num, list_inodes[a].inode_block_pointer[14], (list_indirects[b].entry * blocksize/4 * blocksize/4) + (list_indirects[d].entry + (blocksize/4) * list_indirects[c].entry));
					    }
					  
					  for(int j = 0; j < bitmap_lines; j++)
					    {
					      if(list_bitmaps[j].block_num ==  list_indirects[d].block_ptr)
						{
						  for(int k = 0; k < group_lines; k++)
						    {
						      if(list_bitmaps[j].group_num == list_groups[k].group_block_num)
							{
							  dprintf(file,"INODE < %d > INDIRECT BLOCK < %d > ENTRY < %d >\n", list_inodes[a].inode_num, list_inodes[a].inode_block_pointer[14], (list_indirects[b].entry * blocksize/4 * blocksize/4) + (list_indirects[d].entry + (blocksize/4) * list_indirects[c].entry));
							}
						    }
						}
					      
					    }
					}
				    }
				}
			    }
			}
		    }
		}
	    }

	  else
	    {
	      if(list_inodes[a].inode_block_pointer[i] != 0)
		{

		  if(list_inodes[a].inode_block_pointer[i] > totalnumblocks)
		    {
		      dprintf(file,"INVALID BLOCK < %d > IN INODE < %d > ENTRY < %d >", list_inodes[a].inode_block_pointer[i], list_inodes[a].inode_num, i);
		    }
		  
		  for(int j = 0; j < bitmap_lines; j++)
		    {
		      if(list_inodes[a].inode_block_pointer[i] == list_bitmaps[j].block_num)
			{
			  for(int k = 0; k < group_lines; k++)
			    {
			      if(list_groups[k].group_block_num == list_bitmaps[j].group_num)
				dprintf(file,"UNALLOCATED BLOCK < %d > REFERENCED BY INODE < %d > ENTRY < %d >\n", list_inodes[a].inode_block_pointer[i], list_inodes[a].inode_num, i);
			    }
			}
		    } 
		}
	    }
	}
      
      }        

    int link_match_count = 0;
    //----------link count error--------//
    for(int i = 0; i < inode_lines; i++){
      if(list_inodes[i].link_count != 0){
	link_match_count = 0;

	for(int j = 0; j < directory_lines; j++){
	  if(list_inodes[i].inode_num == list_directories[j].dir_parent)
	    link_match_count++;
	}

	if(link_match_count != list_inodes[i].link_count)
	  dprintf(file,"LINKCOUNT < %d > IS < %d > SHOULD BE < %d >\n", list_inodes[i].inode_num, list_inodes[i].link_count, link_match_count);
      }
    }

    //--------directory entries period-----//
    int found = 0;
    int correct = 0;
    for(int i = 0; i < period_counter; i+=2){
      if(list_directories[i].period_dir_current != list_directories[i].period_dir_parent){
	dprintf(file,"INCORRECT ENTRY IN < %d > NAME < . > LINK TO < %d > SHOULD BE < %d >\n",list_directories[i].period_dir_current, list_directories[i].period_dir_parent, list_directories[i].period_dir_current);
      }
    }


    for(int i = 3; i < period_counter; i+=2){
      found = 0;
      for(int j = 0; j < directory_lines; j++){
	if( (list_directories[j].dir_current == list_directories[i].period_dir_parent) && ((list_directories[j].dir_parent == list_directories[i].period_dir_current) && !!strcmp(list_directories[j].dir_name, "\"..\""))  && !!strcmp(list_directories[j].dir_name, "\".\"")){
	  found = 1;
	}
      }
      if(found == 0){
	for(int j = 0; j < directory_lines; j++){

	  if((list_directories[i].period_dir_current == list_directories[j].dir_parent) && !!strcmp(list_directories[j].dir_name, "\"..\"")  && !!strcmp(list_directories[j].dir_name, "\".\"")){
	    correct = list_directories[j].dir_current;
	  }
	}
	dprintf(file,"INCORRECT ENTRY IN < %d > NAME < .. > LINK TO < %d > SHOULD BE < %d >\n", list_directories[i].period_dir_current, list_directories[i].period_dir_parent, correct);
      }
    }
    
}

