/**
 * @file phonebook_module.c
 *
 * @brief PhoneBook module
 * @author Bobylev Igor,
 * Contact: igor.v.bobylev@gmail.com
 *
 */

#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <asm/uaccess.h>
#include <linux/list.h>

#include <linux/cdev.h>
#include <linux/parport.h>
#include <linux/pci.h>
#include <linux/version.h>

#include "userdata.h"

MODULE_AUTHOR("Bobylev Igor");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Phone book module");

#define DEVICE_NAME "phoneBook"
#define MODULE_PREFIX "<PHONEBOOK DRIVER> "

struct user_data_list {
    user_data_t* data;
    struct list_head list;
};
typedef struct user_data_list user_data_list_t;

static int major_number;
dev_t devNo;
struct class* pClass;
struct device* pDev;

static volatile int is_device_open = 0;
user_data_list_t* g_user_data_list = NULL;

static void init_list(void);
static void free_list(void);
static user_data_list_t* get_next(const user_data_list_t* node);
static user_data_list_t* find(const char* surname, const uint32_t surnamesize);
static void add(user_data_t* data);
static void remove(user_data_list_t* node);

static void init_list(void) {
    g_user_data_list = NULL;
}

static void free_list(void) {
    user_data_list_t* next = NULL;

    while (g_user_data_list) {
        if (NULL != g_user_data_list->data) {
            kfree(g_user_data_list->data);
        }

        next = get_next(g_user_data_list);
        kfree(g_user_data_list);
        g_user_data_list = next;
    }
}

static user_data_list_t* get_next(const user_data_list_t* node) {
    const struct list_head* next = node->list.next;
    if (NULL == next)
        return NULL;

    return (user_data_list_t*)((uint8_t*)next - offsetof(user_data_list_t, list));
}

static user_data_list_t* find(const char* surname, const uint32_t surnamesize) {
    user_data_list_t* node = g_user_data_list;

    while(NULL != node) {
        if ((NULL != node->data) && 
            (surnamesize == node->data->m_surnameSize) &&
            (0 == strcmp(node->data->m_surname, surname))) {
                return node;
        }
        node = get_next(node);
    }

    return NULL;
}

static void add(user_data_t* data) {
    user_data_list_t* node = NULL;

    node = kmalloc(sizeof(user_data_list_t), GFP_KERNEL);
    node->data = data;
    //INIT_LIST_HEAD(&node->list);
    node->list.prev = NULL;
    if (NULL == g_user_data_list)
        node->list.next = NULL;
    else
    {
        node->list.next = &g_user_data_list->list;
        g_user_data_list->list.prev = &node->list;
    }

    g_user_data_list = node;
}

static void remove(user_data_list_t* node) {
    struct list_head* next = NULL;
    struct list_head* prev = NULL;

    if (NULL == node) {
        return;
    }
    
    next = node->list.next;
    prev = node->list.prev;

    if (node == g_user_data_list) {
        if (NULL == next)
            g_user_data_list = NULL;
        else
            g_user_data_list = (user_data_list_t*)((uint8_t*)next - offsetof(user_data_list_t, list));
    }

    if (NULL != next) {
        next->prev = prev;
    }

    if (NULL != prev) {
        prev->next = next;
    }

    if (NULL != node->data) {
        kfree(node->data);
    }
    kfree(node);
}

static user_data_t* parse_user(const char __user* buff, size_t size) {
    user_data_t* data = NULL;
    if (sizeof(user_data_t) > size)
        return NULL;

    data = kmalloc(sizeof(user_data_t), GFP_KERNEL);
    if (NULL == data)
        return NULL;

    memcpy(data, buff, sizeof(user_data_t));

    return data;
}

static int parse_surname(const char __user* buff, const size_t size, char* surname, uint32_t* surnamesize) {
    if (0 == size)
        return 0;

    *surnamesize = (int)(buff[0]);
    if (MAX_SURNAME < (*surnamesize) || (size - 1) < (*surnamesize))
        return 0;

    memcpy(surname, buff + 1, *surnamesize);

    return 1;
}

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static ssize_t device_read(struct file*, char*, size_t, loff_t*);
static ssize_t device_write(struct file*, const char*, size_t, loff_t*);

static struct file_operations fops = {
    .llseek = NULL,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int __init base_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0) {
        printk(MODULE_PREFIX "Registering the character device failed with %d\n", -major_number);
        return major_number;
    }

    devNo = MKDEV(major_number, 0);

    pClass = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(pClass)) {
        printk(MODULE_PREFIX "Failed to create class\n");
        unregister_chrdev_region(devNo, 1);
        return -1;
    }

    printk(MODULE_PREFIX "Class name %s\n", pClass->name);
    
    pDev = device_create(pClass, NULL, devNo, NULL, DEVICE_NAME);
    if (IS_ERR(pDev)) {
        printk(MODULE_PREFIX "Failed to create device\n");
        unregister_chrdev_region(devNo, 1);
        return -1;
    }

    init_list();

    printk(MODULE_PREFIX "Phonebook module is loaded!\n");

    return 0;
}

static void __exit base_exit(void) {
    free_list();
    
    device_destroy(pClass, devNo);
    class_destroy(pClass);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(MODULE_PREFIX "Phonebook module is unloaded!\n");
}

module_init( base_init );
module_exit( base_exit );

static int device_open(struct inode *inode, struct file *file) {
    printk(MODULE_PREFIX "Open device\n");

    if (is_device_open)
        return -EBUSY;
  
    is_device_open = 1;

    return 0;
}

static int device_release(struct inode* inode, struct file* file) {
    printk(MODULE_PREFIX "Close device\n");
    is_device_open = 0;
    return 0;
}

user_data_t* ret = NULL;

static ssize_t device_write(struct file* filp, const char __user *buff, size_t size, loff_t* offset) {
    user_data_t* userdata = NULL;
    user_data_list_t* data = NULL;
    if (0 == size) {
        return -EFAULT;
    }
    ret = NULL;

    if (COMMAND_ADD_USER == buff[0]) {
        userdata = parse_user(buff + 1, size - 1);
        if (NULL == userdata)
            return -EFAULT;

        if (NULL != find(userdata->m_surname, userdata->m_surnameSize))
            return -EINVAL;
        
        add(userdata);
        return size;
    }

    else if (COMMAND_DEL_USER == buff[0]) {
        char surname[MAX_SURNAME];
        uint32_t surnamesize;
        if (0 == parse_surname(buff + 1, size - 1, surname, &surnamesize))
            return -EFAULT;

        data = find(surname, surnamesize);
        if (NULL == data)
            return -EINVAL;

        remove(data);
        return size;
    }

    else if (COMMAND_GET_USER == buff[0]) {
        char surname[MAX_SURNAME];
        uint32_t surnamesize;
        if (0 == parse_surname(buff + 1, size - 1, surname, &surnamesize))
            return -EFAULT;

        data = find(surname, surnamesize);
        if (NULL == data)
            return -EINVAL;
        
        ret = data->data;
        return size;
    }

    return -EFAULT;
}

static ssize_t device_read(struct file* filp, char __user *buff, size_t size, loff_t* offset) {
    if (0 == size) {
        return -EFAULT;
    }
    
    if (NULL != ret) {
        if (sizeof(user_data_t) > size)
            return -EFAULT;

        memcpy(buff, ret, sizeof(user_data_t));

        return sizeof(user_data_t);
    }

    return -EFAULT;
}