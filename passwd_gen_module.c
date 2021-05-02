#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/random.h>
#define BUFFER_LEN 16
#define DEVICE_NAME "Password_Generator"

#define MINIMUM_CHAR_IN_ASCII 0x21
MODULE_LICENSE("GPL");
static dev_t dev = 0;
static struct class *device_file = NULL;
static struct cdev passwd_gen_cdev;
static char passwd[16];

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



static ssize_t read_character_device(struct file* fd, char __user* buffer, size_t size, loff_t* pos)
{
	char byte = '\0';
	char len = BUFFER_LEN;
	char* it = passwd;
	size_t remainder_size = BUFFER_LEN - idx; // 0
	size_t bytes_to_copy = 0;
	while(len != 0)
	{
		byte = get_random_char();
		*it = byte;
		it++;
		len--;	
	}
	bytes_to_copy = (remainder_size <= size) ? remainder_size  : size; // 0
	pr_info(" DUPA bytes_to_copy = %d", bytes_to_copy);
	if(copy_to_user(buffer, passwd, bytes_to_copy))
		return -EPERM;
	pr_info("\n DUPA device read function %s, checK: %d", passwd, (bytes_to_copy == 0) ? 0 : 1);
	idx += bytes_to_copy;
	pr_info("idx = %d\n bytes_to_copy = %d\n reminder_size = %d\n offset = %d\n", idx, bytes_to_copy, remainder_size, *pos);
	return bytes_to_copy;
}

static ssize_t write_character_device(struct file* fd, const char __user* buffor, size_t size, loff_t* loff_ptr)
{
	pr_info("device write fun");
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
	return 0;
}


static void __exit exit_device(void)
{
	printk(KERN_INFO "Device Character Exit \n");
	device_destroy(device_file, dev);
	class_destroy(device_file);
	unregister_chrdev_region(dev, 1);
}
	

module_init(init_device);
module_exit(exit_device);
