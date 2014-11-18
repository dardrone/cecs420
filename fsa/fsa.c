/*
 ============================================================================
 Name	     : proj1.c
 Author      : Darren Jennings
 Version     :
 Copyright   : Copyright 2014
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include "dbg.h"
#include "list.c"
#include <ext2fs/ext2fs.h>

int main(int argc, char *argv[]) {

	//////////////////
	//PRINT ARGS
	//////////////////
	/*int i=0;
	 while(i < argc) {
	 printf("\narg[%d]: %s\n", i, argv[i]);
	 i++;
	 }*/
	int rv, fileDisk;

	struct ext2_super_block *sb = NULL;

	char *diskImage = argv[1];

	// Check to see it exists
	if (diskImage == NULL) {
		fprintf(stderr, "Please check your argument list...\n");
		exit(1);
	}

	fileDisk = open(diskImage, O_RDONLY);
	if (fileDisk == -1) {
		perror("disk_image_file open failed");
		exit(1);
	}

	sb = malloc(sizeof(struct ext2_super_block));

	/*set the file offset to byte 1024 again*/
	if (lseek(fileDisk, 1024, SEEK_SET) != 1024) {
		perror("File seek failed");
		exit(1);
	}

	/*read the whole superblock and load into the ext2_super_block struct*/
	/*assumes the struct fields are laid out in the order they are defined*/
	rv = read(fileDisk, sb, sizeof(struct ext2_super_block));
	if (rv == -1) {
		perror("File read failed");
		exit(1);
	}
	printf("--General File System Information--\n");
	if (rv == 1024) {
		printf("Block size in Bytes: %u\n",
				(sb->s_log_block_size * sb->s_log_block_size) * 1024);
		printf("Total Number of Blocks: %u\n", (sb->s_blocks_count));
		printf("Total Number of Inodes: %u\n", sb->s_inodes_count);
		printf("Number of Free Inodes: %u\n", sb->s_free_inodes_count);
		printf("Disk Size in Bytes: %u\n", (sb->s_blocks_count	* (sb->s_log_block_size * sb->s_log_block_size) * 1024));
		printf("Maximum Number of Blocks Per Group: %u\n", sb->s_blocks_per_group);
		printf("Inode Size in Bytes: %u\n", sb->s_inode_size);
		printf("Number of Inodes Per Group: %u\n", sb->s_inodes_per_group);
		printf("Number of Inode Blocks Per Group: %u\n", (sb->s_inodes_per_group * sb->s_inode_size)/((sb->s_log_block_size * sb->s_log_block_size) * 1024));
		printf("Number of Groups: %d\n", (sb->s_blocks_count + sb->s_blocks_per_group - 1)/sb->s_blocks_per_group);
	}

	printf("\n--Individual Group Information--\n");
	printf("-Group ...-\n");
	printf("Block IDs: ...\n");
	printf("Block Bitmap Block ID: ...\n");
	printf("Inode Bitmap Block ID: ...\n");
	printf("Inode Table Block ID: ...\n");
	printf("Number of Free Blocks: ...\n");
	printf("Number of Free Inodes: ...\n");
	printf("Number of Directories: ...\n");
	printf("Free Block IDs: ...\n");
	printf("Free Inode IDs: ...\n");

	printf("\n--Root Directory Entries--\n");
	printf("Inode: ...\n");
	printf("Entry Length: ...\n");
	printf("Name Length: ...\n");
	printf("File Type: ...\n");
	printf("Name: ...\n");

	/////////////////////////////////
	//
	// Free memory stuff
	//
	/////////////////////////////////

	free(sb);

	//debug("*Finished*");
	return 0;
}
