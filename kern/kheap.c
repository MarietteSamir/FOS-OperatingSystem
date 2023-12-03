#include <inc/memlayout.h>
#include <kern/kheap.h>
#include <kern/memory_manager.h>

// NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)

// Array to keep track of allocated pages in kernel heap ( 0=free , 1=allocated )
unsigned int numOfPages[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE] = {0};

unsigned int NumOfNeededPages; // Number of pages needed for the requested memory allocation

struct Frame_Info *ptr_frame_info;

int NextAllocIndex = 0;

struct Kernel_Heap_info // to keep track of the data of the block allocated made by kmalloc()
{
	int FirstIndex;
	uint32 FirstVA;
	int numOfAllocatedPages;
	unsigned int size;
} AllocatedBlock[(KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE] = {0};

void *kmalloc(unsigned int size)
{
	// TODO: [PROJECT 2023 - MS1 - [1] Kernel Heap] kmalloc()
	//  Write your code here, remove the panic and write your code
	// kpanic_into_prompt("kmalloc() is not implemented yet...!!");

	uint32 virtual_address = KERNEL_HEAP_START;
	int contiguousFreePages = 0; // Counter to keep track of the number of consecutive free pages in the kernel heap

	if (size > (KERNEL_HEAP_MAX - KERNEL_HEAP_START) || size <= 0) // Check if the size is within the range of the kernel heap
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
	// Loop through the heap pages to find a block of free pages to allocate
	for (int j = 0; j < ((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE); j++)
	{
		if (NextAllocIndex >= ((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE))
		{ // Check if there is enough space in the AllocatedBlock array
			cprintf("Kernel heap is full!\n");
			return NULL;
		}
		// Check if the current page is free or allocated
		if (numOfPages[j] == 0)
		{ // A free page is found , add it to the counter
			contiguousFreePages++;
		}
		else
		{ // Interrupted by an allocated page , reset the counter
			contiguousFreePages = 0;
		}
		if (contiguousFreePages == NumOfNeededPages)
		{																					// Found a block of free pages, allocate them
			virtual_address = KERNEL_HEAP_START + ((j - NumOfNeededPages + 1) * PAGE_SIZE);    // 1st VA to be allocated
			for (int i = 0; i < NumOfNeededPages; i++)
			{
				int ret = allocate_frame(&ptr_frame_info);
				if (ret == E_NO_MEM)
				{
					cprintf("No enough memory for page itself!\n");
					return NULL;
				}
				ret = map_frame(ptr_page_directory, ptr_frame_info, (void *)(virtual_address + (i * PAGE_SIZE)), PERM_PRESENT | PERM_WRITEABLE);
				if (ret == E_NO_MEM)
				{
					cprintf("No enough memory for page table!\n");
					free_frame(ptr_frame_info);
					return NULL;
				}
				numOfPages[j - NumOfNeededPages + 1 + i] = 1; // Mark the page as allocated in the numOfPages array
			}
			// Update the AllocatedBlock entry
			AllocatedBlock[NextAllocIndex].FirstIndex = (j - NumOfNeededPages + 1);
			AllocatedBlock[NextAllocIndex].FirstVA = virtual_address;
			AllocatedBlock[NextAllocIndex].numOfAllocatedPages = NumOfNeededPages;
			AllocatedBlock[NextAllocIndex].size = size;
			NextAllocIndex++;
			return (void *)virtual_address;
		}
	}
	// If we get here, there is not enough contiguous free pages
	return NULL;

	// NOTE: Allocation is based on FIRST FIT strategy
	// NOTE: All kernel heap allocations are multiples of PAGE_SIZE (4KB)
	// refer to the project presentation and documentation for details

	// change this "return" according to your answer
	// return NULL;
}

void kfree(void *virtual_address)
{
	// TODO: [PROJECT 2023 - MS1 - [1] Kernel Heap] kfree()

	uint32 VA = (uint32)virtual_address;
	int startIndex = -1;
	int index = -1;

	if (VA < KERNEL_HEAP_START || VA > KERNEL_HEAP_MAX || virtual_address == NULL)
	{
		// The given virtual address is not within the range of the kernel heap.
		cprintf("Invalid Virtual Address!\n");
		return;
	}

	// Check if virtual address is within the range of the allocated block
	for (int i = 0; i < (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE; i++)
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
		unmap_frame(ptr_page_directory, (void *)(AllocatedBlock[index].FirstVA + (i * PAGE_SIZE)));
	}

	// Clear the AllocatedBlock entry
	AllocatedBlock[index].FirstIndex = -1;
	AllocatedBlock[index].FirstVA = 0;
	AllocatedBlock[index].numOfAllocatedPages = 0;
	AllocatedBlock[index].size = 0;

	// you need to get the size of the given allocation using its address
	// refer to the project presentation and documentation for details
}

unsigned int kheap_virtual_address(unsigned int physical_address)
{
	// TODO: [PROJECT 2023 - MS1 - [1] Kernel Heap] kheap_virtual_address()
	//  Write your code here, remove the panic and write your code
	// panic("kheap_virtual_address() is not implemented yet...!!");

	// Round down the physical address to the nearest page boundary
	physical_address = ROUNDDOWN(physical_address, PAGE_SIZE);

	// Check each virtual address in the kernel heap
	for (int i = 0; i < ((KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE); i++)
	{
		unsigned int virtual_address = KERNEL_HEAP_START + (i * PAGE_SIZE);

		uint32 *ptr_table = NULL;
		struct Frame_Info *ptr_frame_info = get_frame_info(ptr_page_directory, (void *)virtual_address, &ptr_table);

		if (ptr_frame_info != NULL)
		{
			unsigned int physaddrtobecompared = to_physical_address(ptr_frame_info);

			if (physaddrtobecompared == physical_address)
			{
				// Return the virtual address corresponding to the given physical address
				return virtual_address;
			}
		}
	}

	// If the physical address is not mapped to any virtual address in the kernel heap, return 0
	return 0;

	// return the virtual address corresponding to given physical_address
	// refer to the project presentation and documentation for details

	// change this "return" according to your answer

	// return 0;
}	

unsigned int kheap_physical_address(unsigned int virtual_address)
{
	// TODO: [PROJECT 2023 - MS1 - [1] Kernel Heap] kheap_physical_address()
	//  Write your code here, remove the panic and write your code
	// panic("kheap_physical_address() is not implemented yet...!!");

	virtual_address = ROUNDDOWN(virtual_address, PAGE_SIZE);

	unsigned int physical_address = 0;

	if (virtual_address >= KERNEL_HEAP_START && virtual_address < KERNEL_HEAP_MAX)
	{
		uint32 *ptr_table = NULL;
		struct Frame_Info *ptr_frame_info = get_frame_info(ptr_page_directory, (void *)virtual_address, &ptr_table);
		if (ptr_frame_info != NULL)
			physical_address = to_physical_address(ptr_frame_info);
	}
	else
	{
		return 0;
	}

	return physical_address;

	// return the physical address corresponding to given virtual_address
	// refer to the project presentation and documentation for details

	// change this "return" according to your answer

	// return 0;
}

//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//
// krealloc():

//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to kmalloc().
//	A call with new_size = zero is equivalent to kfree().


// krealloc is NOT COMPLETELY WORKING YET! Current Evaluation = 30%
void *krealloc(void *virtual_address, uint32 new_size)
{
	// panic("krealloc() is not implemented yet...!!");
	// return NULL;
	int index = -1;
	if (virtual_address == NULL && new_size != 0)
	{
		return kmalloc(new_size);
	}
	if (new_size == 0 && virtual_address != NULL)
	{
		kfree(virtual_address);
		return NULL;
	}
	if (virtual_address == NULL && new_size == 0)
	{
		return NULL;
	}
	// Check if virtual address is within the range of the allocated block
	for (int i = 0; i < (KERNEL_HEAP_MAX - KERNEL_HEAP_START) / PAGE_SIZE; i++)
	{
		if (virtual_address >= (void *)AllocatedBlock[i].FirstVA &&
			virtual_address < (void *)(AllocatedBlock[i].FirstVA + (AllocatedBlock[i].numOfAllocatedPages * PAGE_SIZE)))
		{
			index = i;
			break;
		}
	}
	if (index == -1)
	{
		// The given virtual address is not in a valid allocated block
		return NULL;
	}
	//  If size is less than or equal to the size of the existing memory block, it returns the existing virtual address
	if ((AllocatedBlock[index].numOfAllocatedPages * PAGE_SIZE) >= new_size)
	{
		return virtual_address;
	}
	else
	{
		// kfree(virtual_address);
		return kmalloc(new_size);
	}
}
