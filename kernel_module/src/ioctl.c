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
#include <linux/sched.h>


extern unsigned long delete_object(unsigned long object_id);                                // Defined in core.c
extern struct mutex * get_object_mutex(unsigned long object_id);                            //Defined in core.c
extern unsigned long get_object_size(unsigned long object_id);                              //Defined in core.c
extern void insert_object(unsigned long object_id, unsigned long size, unsigned long pfn);  //Defined in core.c
extern void update_lock_pid(unsigned long object_id, int current_pid);                      //Defined in core.c
extern int get_lock_pid(unsigned long object_id);                                           //Defined in core.c

// If exist, return the data.
long npheap_lock(struct npheap_cmd __user *user_cmd)
{
    //printk(KERN_INFO "ioctl.c::npheap_lock START... PID : %d\n", current->pid); //Remove

    struct npheap_cmd kernel_cmd;

    if(copy_from_user(&kernel_cmd, user_cmd, sizeof(*user_cmd)))
    {
        printk(KERN_ERR "ioctl.c:: copy_from_user failed! \n");
        return -1;
    }

    struct mutex *result = get_object_mutex(kernel_cmd.offset >> PAGE_SHIFT);

    if(result == NULL){
        //printk(KERN_INFO "ioctl.c:: mutex_lock doesn't exist! \n");
        //Create an entry for the new object in the object_map
        insert_object(kernel_cmd.offset >> PAGE_SHIFT,0,-1);
        result = get_object_mutex(kernel_cmd.offset >> PAGE_SHIFT);
        //return -1;
    }
    //printk(KERN_INFO "ioctl.c::npheap_lock Locking... %ld \n", kernel_cmd.offset >> PAGE_SHIFT);
    mutex_lock_killable(result);
    update_lock_pid(kernel_cmd.offset >> PAGE_SHIFT,(int) current->pid);

    //printk(KERN_INFO "ioctl.c::npheap_lock LOCKED... %ld \n", kernel_cmd.offset >> PAGE_SHIFT);

    //printk(KERN_INFO "ioctl.c::npheap_lock END \n");
    return 0;
}     

long npheap_unlock(struct npheap_cmd __user *user_cmd)
{
    //printk(KERN_INFO "ioctl.c::npheap_unlock START... PID : %d \n", current->pid);
    
        struct npheap_cmd kernel_cmd;
    
        if(copy_from_user(&kernel_cmd, user_cmd, sizeof(*user_cmd)))
        {
            printk(KERN_ERR "ioctl.c:: copy_from_user failed! \n");
            return -1;
        }
    
        struct mutex *result = get_object_mutex(kernel_cmd.offset >> PAGE_SHIFT);
    
        if(result == NULL){
            //printk(KERN_ERR "ioctl.c:: mutex_lock doesn't exist! \n");
            return -1;
        }

        //Allow the process to unlock to mutex only if it holds the lock
        //if(get_lock_pid(kernel_cmd.offset >> PAGE_SHIFT) == (int) current->pid) {

            //printk(KERN_INFO "ioctl.c::npheap_unlock UnLocking... %ld \n", kernel_cmd.offset >> PAGE_SHIFT);
            mutex_unlock(result);
            update_lock_pid(kernel_cmd.offset >> PAGE_SHIFT,-1);
            //printk(KERN_INFO "ioctl.c::npheap_unlock UNLOCKED... %ld \n", kernel_cmd.offset >> PAGE_SHIFT);
            return 0;
        //} //NOT A GOOD PRACTICE
       
        //printk(KERN_INFO "ioctl.c::npheap_unlock END \n");
        //return -1;
}

long npheap_getsize(struct npheap_cmd __user *user_cmd)
{
    struct npheap_cmd kernel_cmd;

    if(copy_from_user(&kernel_cmd, user_cmd, sizeof(*user_cmd)))
    {
        printk(KERN_ERR "ioctl.c::npheap_getsize copy_from_user failed! \n");
        return -1;
    }

    return get_object_size(kernel_cmd.offset >> PAGE_SHIFT);
}

long npheap_delete(struct npheap_cmd __user *user_cmd)
{   

    struct npheap_cmd kernel_cmd;

    //printk(KERN_INFO "ioctl.c::npheap_delete \n");
    //printk(KERN_INFO "ioctl.c::npheap_delete user_cmd NULL check :: \n");

    if(user_cmd == NULL){
        //printk(KERN_ERR "ioctl.c::user_cmd object is NULL \n");
        return -1;
    }
    else{
          //printk(KERN_ERR "ioctl.c::user_cmd object is not NULL \n");

          if(copy_from_user(&kernel_cmd, user_cmd, sizeof(*user_cmd)))
          {
              printk(KERN_ERR "ioctl.c:: copy_from_user failed! \n");
              return -1;
          }
    }
    
    unsigned long pfn = -1;

    //printk(KERN_INFO "calling core.c::delete_object with %ld object_id \n", kernel_cmd.offset >> PAGE_SHIFT);
    
    pfn = delete_object(kernel_cmd.offset >> PAGE_SHIFT);
    
    if (pfn != -1){
        //printk(KERN_INFO "ioctl.c::npheap_delete : pfn freed %ld\n", pfn);
        return 0;
    } else {
       // printk(KERN_ERR "npheap_delete : pfn does not exists");
        return -1;
    }

    //printk(KERN_INFO "ioctl.c::npheap_delete END \n");
    return 0;
}

long npheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case NPHEAP_IOCTL_LOCK:
        return npheap_lock((void __user *) arg);
    case NPHEAP_IOCTL_UNLOCK:
        return npheap_unlock((void __user *) arg);
    case NPHEAP_IOCTL_GETSIZE:
        return npheap_getsize((void __user *) arg);
    case NPHEAP_IOCTL_DELETE:
        return npheap_delete((void __user *) arg);
    default:
        return -ENOTTY;
    }
}
