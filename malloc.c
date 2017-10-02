#include<stdio.h>
#include<sys/mman.h>
typedef struct LinkedList LinkedList;

int numOfNodes = 0;
void * base;
struct LinkedList * head;

void *my_buddy_malloc(int size);
void my_free(void *ptr);

struct LinkedList{
	struct LinkedList * next;
	struct LinkedList * prev;
	char dataByte;
};

void *my_buddy_malloc(int size){
	if(numOfNodes == 0){		//base case for initializing malloc
		//make base point to start of memory for allocating
		base = mmap(NULL, 1 << 30, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);
		//now create head node to be equal to base
		head = base;
		//by creating the start of our list, base case will be skipped.
		head -> dataByte = 0b00011110;		//starts with one chunk at 2^30 (1GiB)
		printf("Head DataByte = %i\n",head -> dataByte);
		printf("Head = %p\n", head);
		numOfNodes++;
		my_buddy_malloc(size);			//Call again because we only initialized.
	}
	else{
		struct LinkedList * curr;		//Temp variable
		curr = head;
		printf("Curr (Start) = %p\n",curr);
		//Start general algorithim
		//Size must be divisible by 32
		while(size % 32 != 0){
			size++;						//Gross, ew, but works.
		}
		//Now our size is proportional to our minimum allocation chunk size
		while(curr != NULL){
			if((curr -> dataByte & 0b10000000) == 0){
				printf("Curr DataByte (Free) = %i\n", curr -> dataByte);
				//Enters if when temp is pointing to a free node
				//So now we must check if this free node is of a larger size
				//than the size we want to malloc. We must convert the size to decimal.
				//Use a bit-wise & operation to get that value
				int allocSize =  1 << (curr -> dataByte);			//Should contain alloc size.
				int allocDataByte = curr -> dataByte & 0b01111111;
				printf("Free Chunk AllocSize(%i) = %i\n",allocDataByte,allocSize);
				printf("Alloc (Start) = %i\n", allocDataByte);
				if(size <= allocSize){
					//This means that size is less than the free chunk.
					printf("Mallocing in free chunk...\n");
					//Here is where we implement our buddy algorithm, dividing the chunk.
					//Must re-encode the chunk's dataByte to match new sizes.
					//Add in new chunks when divided and add to the List
					while(allocSize/2 >= size){		//Size is divisible by 32
						allocSize >>= 1;
						allocDataByte--;		//Contains the Binary Value of allocSize (2^n)
						curr -> dataByte = allocDataByte;
									//Create buddy at free address with allocSize offset
						//curr -> next = (LinkedList*){NULL, curr, allocDataByte};
						curr -> next = ((char*)curr + allocSize);
						printf("Curr = %p\n",curr);
						printf("Curr.Next = %p\n",(curr -> next));
						printf("Curr.Next - Curr = %lu\n",(size_t)(curr -> next) - (size_t)curr);
						printf("Created Buddy Node of size %i...\n", allocSize);
                                                struct LinkedList *temp = curr;
						curr = curr -> next;
						curr -> dataByte = allocDataByte;
                                                curr -> next = NULL;
                                                curr -> prev = temp;
						printf("Next DataByte = %i\n", curr -> dataByte);
					}
					curr -> dataByte |= 1 << 7;		//Sets alloc bit to 1
					printf("Curr Binary DataByte = %i\n", curr -> dataByte);
					break;		//#WE OUT HERE
				}
			}
			curr = curr -> next;				//Parse through the list until you reach the end
 		}
 		printf("Malloc'd DataByte = %i\n",curr -> dataByte & 0b01111111);
		return curr;
	}
}	

void my_free(void *ptr){
	struct LinkedList* curr = head;
}

int main(int argc, char *argv[]){
	printf("Malloc 1 (Head 64)= %p\n", my_buddy_malloc(64));
	printf("Global Head = %p\n\n",head);
	printf("Malloc 2 (Curr 128)= %i\n", my_buddy_malloc(128));
	printf("Global Head = %i\n\n",head);

	return 0;
}
