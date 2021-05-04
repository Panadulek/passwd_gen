#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/random.h>
#include <linux/slab.h>
#define DEVICE_NAME "passwd_gen"
#define MINIMUM_CHAR_IN_ASCII 0x21
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jordan_Jazbor");
static size_t BUFFER_LEN = 16;
static dev_t dev = 0;
static struct class *device_file = NULL;
static struct cdev passwd_gen_cdev;
static char *passwd = NULL;
static int idx = 0;
static char get_random_char(void)
{

	char byte = '\0';
	while(byte < MINIMUM_CHAR_IN_ASCII)
	{
		get_random_bytes(&byte, sizeof(byte));
		byte %= 0x7F;
	}
	return byte;
}
static char get_new_buffer_len(char* buffer)
{
	size_t len = strlen(buffer);
	char *it = NULL;
	int multiplier = 1;
	int err_flag = 0;
	char ret = 0;
	if(len > 2)
		err_flag = 1;
	while(len != 0 && !err_flag)
	{
		it = buffer + (len - 1);
		if(*it < '0' || *it > '9')
		{
			err_flag = 1;
			break;
		}
		ret += ((*it - '0') * multiplier);
		len--;
		multiplier *= 10;
	}
	if(err_flag)
		ret = BUFFER_LEN;
	return ret;
}
static ssize_t read_character_device(struct file* fd, char __user* buffer, size_t size, loff_t* pos)
{
	char byte = '\0';
	int len = BUFFER_LEN;
	char* it = passwd;
	pr_info("odczytuje");
	while(len != 0)
	{
		byte = get_random_char();
		*it = byte;
		it++;
		len--;	
	}
	size = (BUFFER_LEN - *pos < size) ? (BUFFER_LEN - *pos)  : size; // 0
	if(copy_to_user(buffer, passwd, size))
		return -EPERM;
	*pos += size;
	return size;
}
static ssize_t write_character_device(struct file* fd, const char __user* user_buffer, size_t size, loff_t* offset)
{
	char kern_buffer[32];
	size = (sizeof(kern_buffer) - *offset < size) ? (sizeof(kern_buffer) - *offset) : size;
	if(size <= 0)
		return 0;
	if(copy_from_user(kern_buffer, user_buffer, size - 1))
	{
		return -EFAULT;
	}
	BUFFER_LEN = get_new_buffer_len(kern_buffer);
	
	*offset += size;
	return size;
}
static int open_character_device(struct inode* inodePtr, struct file * fd)
{
	
	pr_info("device open fun");
	idx = 0;
	return 0;
}
static int release_character_device(struct inode* inodePtr, struct file *fd)
{
	pr_info("device realese fun");
	return 0;
}
struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = read_character_device,
	.write = write_character_device,
	.open = open_character_device,
	.release = release_character_device
};
static int __init init_device(void)
{
	printk(KERN_INFO "Operation has been initialization");
	if(alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0)
	{	
		printk(KERN_INFO "Alloc_cherdev_region couldnt allocate dev number properly \n");
		return -1;
	}
	else
	{
		printk(KERN_INFO "Alloc_cherdev_region allocated dev number properly %d\n", dev);
	}
	printk(KERN_INFO "cdev init\n");
	cdev_init(&passwd_gen_cdev, &fops);

	if((cdev_add(&passwd_gen_cdev, dev, 1)) < 0)
	{
		printk(KERN_INFO "cdev_add couldnt connect dev with passwd_gen_cdev\n");
		unregister_chrdev_region(dev, 1);
	}
	else
	{
		printk(KERN_INFO "cdev_add connected dev with passwd_gen_cdev \n");
	}
	if((device_file = class_create(THIS_MODULE, DEVICE_NAME)) == NULL)
	{	unregister_chrdev_region(dev, 1);
		printk(KERN_INFO "class_create couldnt create class struct \n");
		return -1;
	}
	else
	{
		printk(KERN_INFO "class_create created class struct \n");
	}
	if((device_create(device_file, NULL, dev, NULL, DEVICE_NAME)) == NULL)
	{
		class_destroy(device_file);
		printk(KERN_INFO "class couldnt create device \n");
	}
	else
	{
		printk(KERN_INFO "class created device \n");
	}
	passwd = (char *)kmalloc(BUFFER_LEN * sizeof(char), GFP_KERNEL);
	return 0;
}
static void __exit exit_device(void)
{
	printk(KERN_INFO "Device Character Exit \n");
	device_destroy(device_file, dev);
	class_destroy(device_file);
	unregister_chrdev_region(dev, 1);
	kfree(passwd);
}
module_init(init_device);
module_exit(exit_device);
