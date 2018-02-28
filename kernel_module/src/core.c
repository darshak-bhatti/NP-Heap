// Project 1: Darshak Harisinh Bhatti, dbhatti; Saloni Desai, sndesai;
//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "npheap.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>

extern struct miscdevice npheap_dev;

struct object_map
{
	unsigned long object_id; 			//Object_id is the offset passed by the process
	unsigned long pfn; 					//The page frame number that the object_id points to
	unsigned long size;   				//Number of pages in the object
	unsigned long reference_count; 		//Reference count for object_id
	int lock_pid;						//Current process locking the object
	struct mutex object_mutex; 			//Mutex for the object_id
	struct object_map *next, *prev; 	//Next and previous pointers
};

static struct object_map *head = NULL;

//Insert object in object_map
void insert_object(unsigned long object_id, unsigned long size, unsigned long pfn)
{

	//printk(KERN_INFO "insert_bject obj : %ld, size : %ld, pfn : %ld",object_id, size, pfn);
	if(head == NULL)
	{
		//Create the first object
		head = (struct object_map *) kmalloc(sizeof(struct object_map),GFP_KERNEL);
		head->object_id = object_id;
		head->size = size;	
		head->pfn = pfn;
		head->reference_count = 1;
		head->lock_pid = -1;
		head->prev = NULL;
		head->next = NULL;

		//printk(KERN_INFO "Initializing mutex_lock \n"); //Remove
		// Initialize the lock
		mutex_init(&(head->object_mutex));
		//printk(KERN_INFO "Initialized mutex_lock \n"); //Remove
		return;
	}

	struct object_map *iterator = head;
	while(iterator->next != NULL)
		iterator = iterator->next;

	struct object_map *temp = (struct object_map *) kmalloc(sizeof(struct object_map),GFP_KERNEL);
	temp->object_id = object_id;
	temp->size = size;
	temp->pfn = pfn;	
	temp->reference_count = 1;
	temp->prev = iterator;
	temp->lock_pid = -1;
	temp->next = NULL;

	//printk(KERN_INFO "Initializing mutex_lock \n"); //Remove
	// Initialize the lock
	mutex_init(&(temp->object_mutex));
	//printk(KERN_INFO "Initialized mutex_lock \n"); //Remove
	iterator->next = temp;

	return;
}


//Search for object in object_map
struct object_map* search_object(unsigned long object_id)
{
	//printk(KERN_INFO "search_bject obj : %ld",object_id); //Remove
	//List is empty
	if(head == NULL)
		return NULL;

	struct object_map *iterator = head;

	while(iterator != NULL)
	{
		if(iterator->object_id == object_id){
			//printk(KERN_INFO "FFFound : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",iterator->object_id, iterator->size, iterator->pfn, iterator->reference_count);
			//printk(KERN_INFO "core.c::Returning search object \n");
			return iterator;
		}
		iterator = iterator->next;
	}

	//No object with object_id exists in the list
	return NULL;
}


//Returns the size of the object if it exists
unsigned long get_object_size(unsigned long object_id)
{
	//printk(KERN_INFO "search_bject obj : %ld",object_id); //REmove
	//List is empty
	if(head == NULL)
		return 0;

	struct object_map *iterator = head;

	while(iterator != NULL)
	{
		if(iterator->object_id == object_id){
			//printk(KERN_INFO "core.c::get_obj_size Found : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",iterator->object_id, iterator->size, iterator->pfn, iterator->reference_count);
			return iterator->size;
		}
		iterator = iterator->next;
	}

	//No object with object_id exists in the list
	return 0;
}

//Return the mutex of the object
struct mutex * get_object_mutex(unsigned long object_id)
{
	//printk(KERN_INFO "search_bject obj : %ld",object_id); //remove
	//List is empty
	if(head == NULL)
		return NULL;

	struct object_map *iterator = head;

	while(iterator != NULL)
	{
		if(iterator->object_id == object_id){
			//printk(KERN_INFO "core.c:: get_object_mutex Found : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",iterator->object_id, iterator->size, iterator->pfn, iterator->reference_count);
			return (&(iterator->object_mutex));
		}
		iterator = iterator->next;
	}

	//No object with object_id exists in the list
	return NULL;
}

//Update the lock pid
void update_lock_pid(unsigned long object_id, int current_pid)
{
	//printk(KERN_INFO "update_lock_pid : %ld",object_id); //Remove
	//List is empty
	if(head == NULL)
		return;

	struct object_map *iterator = head;

	while(iterator != NULL)
	{
		if(iterator->object_id == object_id){
			//printk(KERN_INFO "update_lock_pid FFFound : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",iterator->object_id, iterator->size, iterator->pfn, iterator->reference_count);
			//printk(KERN_INFO "core.c::update_lock_pid \n");
			iterator->lock_pid = current_pid;
			return;
		}
		iterator = iterator->next;
	}

	//No object with object_id exists in the list
	return;
}

//Get_lock_pid 
int get_lock_pid(unsigned long object_id)
{
	//printk(KERN_INFO "get_lock_pid : %ld",object_id); //Remove
	//List is empty
	if(head == NULL)
		return -1;

	struct object_map *iterator = head;

	while(iterator != NULL)
	{
		if(iterator->object_id == object_id){
			//printk(KERN_INFO "get_lock_pid FFFound : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",iterator->object_id, iterator->size, iterator->pfn, iterator->reference_count);
			//printk(KERN_INFO "core.c::get_lock_pid \n");
			return iterator->lock_pid;
			
		}
		iterator = iterator->next;
	}

	//No object with object_id exists in the list
	return -1;
}



//Delete object from object_map
unsigned long delete_object(unsigned long object_id)
{
	//printk(KERN_ERR "core.c::Delete object %ld from object_map \n", object_id); //Remove
	unsigned long pfn;
	
	if(head == NULL){
		//printk(KERN_ERR "core.c::HEAD IS NULL\n");
		return -1;

	}
		

	//Only node	
	if(head->next == NULL){
		if(head->object_id == object_id){
			pfn = head->pfn;

			//printk(KERN_INFO "core.c::Only node Delete - Decreasing reference count \n"); //Remove
			head->reference_count--;
			//printk(KERN_INFO "Only node Delete_Found : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",head->object_id, head->size, head->pfn, head->reference_count);
			if(head->reference_count == 0){

				//printk(KERN_INFO "core.c ::Only node Deleting the object found\n");
				// Destroy the lock
				mutex_destroy(&(head->object_mutex));
				kfree(head);
				head = NULL;

				
				//Delete the object from kernerl memory
				if(pfn != -1){
					//printk(KERN_INFO "core.c :: Only node Deleting the object found\n");
					void * kernel_pointer = phys_to_virt(pfn << PAGE_SHIFT);
					kfree(kernel_pointer);

				}
			}
			return pfn;
		}

//printk(KERN_ERR "Only node Delete_Object from Object_table::The object %ld doesn't exist \n",object_id); //Remove
		return -1;
	}
	
	struct object_map *iterator = head;
	while(iterator != NULL) {

		if(iterator->object_id == object_id)
		{
			struct object_map *temp = iterator;

			//If it is the last node in the list to be deleted
			if(iterator->next != NULL)
				iterator->next->prev = iterator->prev;

			//If it is the first node in the list to be deleted	
			if(iterator->prev != NULL)	
				iterator->prev->next = iterator->next;

			pfn = temp->pfn;
			//printk(KERN_INFO "core.c::Delete - Decreasing reference count \n"); //Remove
			iterator->reference_count--;
			//printk(KERN_INFO "Delete_Found : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",iterator->object_id, iterator->size, iterator->pfn, iterator->reference_count);
			if(iterator->reference_count == 0){

				//printk(KERN_INFO "core.c :: Deleting the object found\n");
				// Destroy the lock
				mutex_destroy(&(temp->object_mutex));
				kfree(temp);

				if(pfn != -1){
					//printk(KERN_INFO "core.c :: Deleting the kernel memory found\n");
					//Delete the object from kernel memory
					void * kernel_pointer = phys_to_virt(pfn << PAGE_SHIFT);
					kfree(kernel_pointer);

				}
				
			}

			return pfn;
		}

		iterator = iterator->next;
	}
	
	//Object with object_id doesn't exist in the structure
	//printk(KERN_ERR "Delete_Object from Object_table::The object %ld doesn't exist \n",object_id); //Remove
	return -1;
}


int npheap_mmap(struct file *filp, struct vm_area_struct *vma)
{

	/*Remove
	printk(KERN_INFO "NPHEAP_MMAP_IMPL \n");
	
	printk(KERN_INFO "VM_START -> %ld \n ", vma->vm_start);
    printk(KERN_INFO "VM_END -> %ld \n", vma->vm_end);
	printk(KERN_INFO "VM_OFFSET -> %ld \n", vma->vm_pgoff);
	printk(KERN_INFO "f_pos -> %ld \n", filp->f_pos);*/

    //Get the chunk size of memory to be allocated
	unsigned long size;
	size = vma->vm_end - vma->vm_start;

    //Check if the object has already been created
    struct object_map *search_result = search_object(vma->vm_pgoff);
    if((search_result != NULL) && (search_result->pfn != -1) )
    {
		//printk(KERN_INFO "core.c::Search result pfn != -1 \n");
		//Increment reference_count for object_id
		search_result->reference_count++;

        //Create page tables
        if (remap_pfn_range(vma, vma->vm_start, search_result->pfn,search_result->size,vma->vm_page_prot)) {
            //printk(KERN_ERR "REMAP_PFN_RANGE ERROR \n");
            return -EAGAIN; 
        }

		//printk(KERN_INFO "MMAP Increased ref count : obj : %ld, size : %ld, pfn : %ld, ref_count : %ld \n",search_result->object_id, search_result->size, search_result->pfn, search_result->reference_count);
		
        return 0;
	}
	
	
	//printk(KERN_INFO "core.c::Allocate kernel memory \n");
    //Allocate continuous kernel space memory
	void * kernel_pointer ;
	kernel_pointer = kmalloc(size, GFP_KERNEL);

	//printk(KERN_INFO "core.c::Get page frame number \n");
    //Get the page frame number
    unsigned long pfn = virt_to_phys(kernel_pointer) >> PAGE_SHIFT;

	//printk(KERN_INFO "core.c::Create page table \n");
    //Create page tables
    if (remap_pfn_range(vma, vma->vm_start, pfn,size,vma->vm_page_prot)) {
        //printk(KERN_ERR "REMAP_PFN_RANGE ERROR \n");
        return -EAGAIN; 
    }

	if((search_result != NULL) && (search_result->pfn == -1)){
		//printk(KERN_INFO "core.c::Search result has pfn -1\n");
		search_result->pfn = pfn;
		search_result->size = size;
		search_result->reference_count = 1;
	} 
	else {

		//Create an entry for the new object in the object_map
		insert_object(vma->vm_pgoff,size,pfn);
	}
	
	/*Remove
    printk(KERN_INFO "2VM_START -> %ld \n ", vma->vm_start);
    printk(KERN_INFO "2VM_END -> %ld \n", vma->vm_end);
    printk(KERN_INFO "2VM_OFFSET -> %ld \n", vma->vm_pgoff << PAGE_SHIFT);

    printk(KERN_INFO "REMAP_PFN_RANGE WORKED \n");*/
    return 0;
}

int npheap_init(void)
{
    int ret;
	//printk(KERN_INFO "NPHEAP_INIT! \n");
    if ((ret = misc_register(&npheap_dev)))
        printk(KERN_ERR "Unable to register \"npheap\" misc device\n");
    else
        printk(KERN_ERR "\"npheap\" misc device installed\n");
    return ret;
}

void npheap_exit(void)
{
	//printk(KERN_INFO "NPHEAP_EXIT! \n");
    misc_deregister(&npheap_dev);
}

