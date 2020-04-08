/*Anemmeabasi Bassey
1032523, CIS3110 W20
Assignment 3, Task 2*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 256
#define PAGES 256
#define FRAME_SIZE 256
#define FRAMES 256
#define TLB_SIZE 16
#define OFFSET_MASK 255
#define OFFSET_BIT 8
#define PAGE_MASK 255

int page_table[PAGES];
int frame_table[FRAMES];
int physical_memory[FRAMES][FRAME_SIZE];
int tlb[TLB_SIZE][2]; //[0] for pages, [1] for frames;
int virtual;
int next_frame = 0;
int next_page = 0;
int entries = 0; //counter for tlb entries
int faults = 0; //counter for page faults
int addresses = 0; //counter for addresses read
int hits = 0; //count for tlb hits

char backer[PAGE_SIZE];
char line[10];

FILE * backstore_file;
FILE * fp;

void readBackStore(int page_number);
void addToTLB(int page_number, int frame_number);
void getPhysical(int virtual);



int main(int argc, char * argv[])
{
	char * address_file;
	
	if((backstore_file = fopen("BACKING_STORE.bin", "rb")) == NULL) 
	{
		fprintf(stderr, "Backing Store file doesnt exist\n");
		exit(EXIT_FAILURE);
	}
	
	//get and read in addresses.txt
	if(argc < 2)
	{
		fprintf(stderr, "Filename missing\n");
		exit(EXIT_FAILURE);
	}
	
	address_file = argv[1];
	
	if((fp = fopen(address_file, "r")) == NULL) 
	{
		fprintf(stderr, "Invalid filename. %s does not exist\n", argv[1]);
		exit(EXIT_FAILURE);
	}
	
	//read in adresses line by line
	while(fgets(line, 10, fp) != NULL)
	{
		virtual = atoi(line);
		
		getPhysical(virtual);
		
		addresses++;
	}
	
	//printf("Number of Translated Addresses = %d\n", addresses);
	//printf("Page Faults = %d\n", faults);
	fprintf(stderr, "Page Fault Rate = %.3f\n", (double)faults/addresses);
	//printf("TLB Hits = %d\n", hits);
	fprintf(stderr, "TLB Hits Rate = %.3f\n", (double)hits/addresses);
	
	fclose(fp);
	fclose(backstore_file);
	
	return 0;
}

void getPhysical(int virtual)
{
	int page = 0;
	int frame = -1;
	int offset = 0;
	int physical = 0;
	int value = 0;
	
	page = ((virtual >> OFFSET_BIT) & PAGE_MASK);
	offset = (virtual & OFFSET_MASK);
	
	//look in tlb for a match
	int i;
	for(i = 0; i < TLB_SIZE; i++)
	{
		if(tlb[i][0] == page)
		{
			frame = tlb[i][1];
			hits++;
		}
	}
	
	//if frame not found in tlb
	if(frame == -1)
	{
		//check in page table
		int i;
		for(i = 0; i < next_page; i++)
		{
			if(page_table[i] == page) frame = frame_table[i];
		}
		
		//if still not found, get from back store
		if(frame == -1)
		{
			readBackStore(page);
			faults++;
			frame = next_frame - 1;
		}
	}
	
	addToTLB(page, frame);
	physical = (frame << OFFSET_BIT)|offset;
	value = physical_memory[frame][offset];
	//printf("frame: %d, offset: %d\n", frame, offset);
	
	printf("Virtual address: %d Physical address: %d Value: %d\n", virtual, physical, value);
	
}

void addToTLB(int page_number, int frame_number)
{
	//check if already in table
	int i;
	for(i = 0; i < entries; i++)
	{
		if(tlb[i][0] == page_number) break;
	}
	
	//if index is equal to entries
	if (i == entries)
	{
		//if there is still space, insert at the end, else move
		//everything over then insert
		if(entries < TLB_SIZE)
		{
			tlb[entries][0] = page_number;
			tlb[entries][1] = frame_number;
		} else {
			for(i = i; i < TLB_SIZE - 1; i++)
			{
				tlb[i][0] = tlb[i+1][0];
				tlb[i][1] = tlb[i+1][1];
			}
			
			tlb[entries-1][0] = page_number;
			tlb[entries-1][1] = frame_number;
		}
	}//if index is not equal to entries
	 else {	
	 	for(i = i; i < entries - 1; i++)
	 	{
	 		tlb[i][0] = tlb[i+1][0];
			tlb[i][1] = tlb[i+1][1];
	 	}
	 	
	 	if(entries < TLB_SIZE)
	 	{
	 		tlb[entries][0] = page_number;
			tlb[entries][1] = frame_number;
	 	} else {
	 		tlb[entries-1][0] = page_number;
			tlb[entries-1][1] = frame_number;
	 	}
	}
	
	if(TLB_SIZE > entries) entries++;
}

void readBackStore(int page_number)
{
	if(fseek(backstore_file, page_number * PAGE_SIZE, SEEK_SET) != 0)
	{
		fprintf(stderr, "error with backing store file - cannot go to beginning \n");
		exit(EXIT_FAILURE);
	}
	
	if(fread(backer, sizeof(signed char), PAGE_SIZE, backstore_file) == 0)
	{
		fprintf(stderr, "error with backing store file - cannot read from file \n");
		exit(EXIT_FAILURE);
	}
	
	
	//load into first available frame
	int i;
	for(i = 0; i < PAGE_SIZE; i++)
	{
		physical_memory[next_frame][i] = backer[i];
	}
	
	page_table[next_page] = page_number;
	frame_table[next_page] = next_frame;
	
	next_frame++;
	next_page++;
}
