
#include <inc/lib.h>

// malloc()
//	This function use BEST FIT strategy to allocate space in heap
//  with the given size and return void pointer to the start of the allocated space

//	To do this, we need to switch to the kernel, allocate the required space
//	in Page File then switch back to the user again.
//
//	We can use sys_allocateMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls allocateMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the allocateMem function is empty, make sure to implement it.

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

// Array to keep track of allocated pages in user heap ( 0=free , 1=allocated )
unsigned int numOfPages[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE] = {0};

unsigned int NumOfNeededPages; // Number of pages needed for the requested memory allocation

int NextAllocIndex = 0;

struct User_Heap_info // to keep track of the data of the block allocated made by malloc()
{
	int FirstIndex;
	uint32 FirstVA;
	int numOfAllocatedPages;
	unsigned int size;
} AllocatedBlock[(USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE] = {0};

void *malloc(uint32 size)
{
	// TODO: [PROJECT 2023 - MS2 - [2] User Heap] malloc() [User Side]

	uint32 virtual_address = USER_HEAP_START;
	int contiguousFreePages = 0;  // Counter to keep track of the number of consecutive free pages in the user heap
	int bestFitIndex = -1;
	int bestFitBlock = ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE) + 1;   // Initialized to the max possible size

	if (size > (USER_HEAP_MAX - USER_HEAP_START) || size <= 0)   // Check if the size is within the range of the user heap
	{
		cprintf("Invalid Size!\n");
		return NULL;
	}

	// Calculate the number of pages required for the allocation
	if (size % PAGE_SIZE == 0)
	{
		NumOfNeededPages = size / PAGE_SIZE;
	}
	else if (size % PAGE_SIZE != 0)
	{
		NumOfNeededPages = (size / PAGE_SIZE) + 1;
	}

	if (sys_isUHeapPlacementStrategyBESTFIT())
	{
		// Loop through the heap pages to find the best-fit block of free pages to allocate
		for (int j = 0; j < ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE); j++)
		{
			if (NextAllocIndex >= ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE))
			{ // Check if there is enough space in the AllocatedBlock array
				cprintf("User heap is full!\n");
				return NULL;
			}
			// Check if the current page is free or allocated
			if (numOfPages[j] == 0)
			{ // A free page is found, add it to the counter
				contiguousFreePages++;
			}
			else
			{ // Interrupted by an allocated page, check if this is the best fit so far
				if (contiguousFreePages >= NumOfNeededPages && contiguousFreePages < bestFitBlock)
				{
					bestFitIndex = j - contiguousFreePages;
					bestFitBlock = contiguousFreePages;
				}
				contiguousFreePages = 0;    // Reset the counter
			}
		}
		// Check if the final block of free pages is the best fit
		if (contiguousFreePages >= NumOfNeededPages && contiguousFreePages < bestFitBlock)
		{
			bestFitIndex = ((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE) - contiguousFreePages;
			bestFitBlock = contiguousFreePages;         
		}
	}
	if (bestFitIndex == -1 || bestFitBlock == (((USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE) + 1))
	{ // If we couldn't find a best-fit block, return NULL
		return NULL;
	}

	virtual_address = USER_HEAP_START + (bestFitIndex * PAGE_SIZE);    // 1st VA to be allocated
	sys_allocateMem(virtual_address, size);          // Allocate the best-fit block of pages
	for (int i = 0; i < NumOfNeededPages; i++)
	{
		numOfPages[bestFitIndex + i] = 1;    // Mark the page as allocated in the numOfPages array
	}

	// Update the AllocatedBlock entry
	AllocatedBlock[NextAllocIndex].FirstIndex = bestFitIndex;
	AllocatedBlock[NextAllocIndex].FirstVA = virtual_address;
	AllocatedBlock[NextAllocIndex].numOfAllocatedPages = NumOfNeededPages;
	AllocatedBlock[NextAllocIndex].size = size;
	NextAllocIndex++;

	return (void *)virtual_address;

	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_allocateMem to invoke the Kernel for allocation
	// 	4) Return pointer containing the virtual address of allocated space,
	//

	// This function should find the space of the required range
	//  *** ON 4KB BOUNDARY ******** //

	// Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy
}

void *smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	// Write your code here, remove the panic and write your code
	panic("smalloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement BEST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	// This function should find the space of the required range
	//  ******** ON 4KB BOUNDARY ******************* //

	// Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	// change this "return" according to your answer
	return 0;
}

void *sget(int32 ownerEnvID, char *sharedVarName)
{
	// Write your code here, remove the panic and write your code
	panic("sget() is not implemented yet...!!");

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement BEST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	// This function should find the space for sharing the variable
	//  ******** ON 4KB BOUNDARY ******************* //

	// Use sys_isUHeapPlacementStrategyBESTFIT() to check the current strategy

	// change this "return" according to your answer
	return 0;
}

// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from page file and main memory then switch back to the user again.
//
//	We can use sys_freeMem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls freeMem(struct Env* e, uint32 virtual_address, uint32 size) in
//		"memory_manager.c", then switch back to the user mode here
//	the freeMem function is empty, make sure to implement it.

void free(void *virtual_address)
{
	// TODO: [PROJECT 2023 - MS2 - [2] User Heap] free() [User Side]
	//  Write your code here, remove the panic and write your code
	// panic("free() is not implemented yet...!!");

	uint32 VA = (uint32)virtual_address;
	int startIndex = -1;
	int index = -1;

	if (VA < USER_HEAP_START || VA > USER_HEAP_MAX || virtual_address == NULL)
	{
		// The given virtual address is not within the range of the user heap.
		cprintf("Invalid Virtual Address!\n");
		return;
	}
	// Check if virtual address is within the range of the allocated block
	for (int i = 0; i < (USER_HEAP_MAX - USER_HEAP_START) / PAGE_SIZE; i++)
	{
		if (virtual_address >= (void *)AllocatedBlock[i].FirstVA &&
			virtual_address < (void *)(AllocatedBlock[i].FirstVA + (AllocatedBlock[i].numOfAllocatedPages * PAGE_SIZE)))
		{
			startIndex = AllocatedBlock[i].FirstIndex;
			index = i;
			break;
		}
	}
	if (startIndex == -1)
	{
		// The given virtual address is not in a valid allocated block
		return;
	}
	// Free the pages allocated to the block
	for (int i = 0; i < AllocatedBlock[index].numOfAllocatedPages; i++)
	{
		numOfPages[startIndex + i] = 0;
	}

	sys_freeMem((uint32)AllocatedBlock[index].FirstVA, AllocatedBlock[index].size);

	// Clear the AllocatedBlock entry
	AllocatedBlock[index].FirstIndex = -1;
	AllocatedBlock[index].FirstVA = 0;
	AllocatedBlock[index].numOfAllocatedPages = 0;
	AllocatedBlock[index].size = 0;

	//  you should get the size of the given allocation using its address
	//  you need to call sys_freeMem()
	//  refer to the project presentation and documentation for details
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//

//=============
// [1] sfree():
//=============
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void *virtual_address)
{
	// TODO: [PROJECT 2019 - BONUS4] Free Shared Variable [User Side]
	//  Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");

	//	1) you should find the ID of the shared variable at the given address
	//	2) you need to call sys_freeSharedObject()
}

//===============
// [2] realloc():
//===============

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_moveMem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		which switches to the kernel mode, calls moveMem(struct Env* e, uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
//		in "memory_manager.c", then switch back to the user mode here
//	the moveMem function is empty, make sure to implement it.

void *realloc(void *virtual_address, uint32 new_size)
{
	// TODO: [PROJECT 2019 - BONUS3] User Heap Realloc [User Side]
	//  Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}
