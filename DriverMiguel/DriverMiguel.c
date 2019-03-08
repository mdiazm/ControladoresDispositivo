/*

    DriverMiguel.c

    3 drivers are described: 

    - led: to control the state of the keyboard LEDs.
    - fibonacci: to obtain Fibonacci numbers. That will have a limit of 256 numbers.
    - save: espacio dedicado de 64K para almacenar/recuperar información (bytes).

    Febrero 2019
    Miguel Díaz Medina

*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "DriverMiguel"
#define DRIVER_CLASS "DriverMiguelClass"
#define NUM_DEVICES 3
#define MEMORY_SIZE 65536
#define KB_DATA 0x60
#define LED_CMD 0xED

static dev_t major_minor = -1;
static int num_open = 0;
static struct class *driverClass = NULL;
static struct device_data {
    const char name[64];
    char data[MEMORY_SIZE];
    int data_size;
} dev_names[NUM_DEVICES] = {
    [0] = {"led", "", 0},
    [1] = {"fibonacci", "", 0},
    [2] = {"save", "", 0}
};

int minor;

/* Definition of the methods for all devices. */
static int open(struct inode *inode, struct file * file) {
    num_open++;
    pr_info("DriverMiguel has been opened %i times", num_open);

    minor = iminor(inode);

    file->private_data = &minor;

    pr_info("The device opened has been /dev/%s", dev_names[minor].name);

    return 0;
}

static int release(struct inode *inode, struct file * file){
    pr_info("DriverMiguel release method");

    return 0;
}

/* Definition of the methods for each device. */

/********************* LED *************************/
static ssize_t led_read(struct file *file, char __user *buffer, size_t count, loff_t *f_pos){

    return 0;
}

static int kb_ready_for_input(void){
    int reps = 1000000, status;

    do 
    {
        status = inb(0x64);
    } while((status & 2) && reps--);

    return (reps != 0);
}

static ssize_t led_write(struct file *file, const char __user *buffer, size_t count, loff_t *f_pos){

    char data[64];

    if(copy_from_user(data, buffer, count))
        return -EINVAL;

    int led;
    led = data[0] & 0x07; /* Mask the rest of the bits. */

    if(!kb_ready_for_input()) return -EBUSY;

    outb(LED_CMD, KB_DATA); /* Send Led-write command*/

    if(!kb_ready_for_input()) return -EBUSY;

    outb(led, KB_DATA);

    return count;
}

/********************** FIBONACCI ************************/
static ssize_t fibonacci_read(struct file *file, char __user *buffer, size_t count, loff_t *f_pos){
    if(*f_pos >= dev_names[1].data_size)
            return 0;

    if(*f_pos + count > dev_names[1].data_size)
        count = dev_names[1].data_size - *f_pos;

    if(copy_to_user(buffer, dev_names[1].data, count))
        return -EFAULT; 

    *f_pos += count;

    return count;
}

static int pow(int base, int _exp){
    int sum = 1, i;

    for(i = 0; i < _exp; i++){
        sum *= base;
    }

    return sum;
}

static int count_diviter(int n){
    int r = 1;
    while(n > 9){
        n /= 10;
        r++;
    }

    return r;
}

static ssize_t fibonacci_write(struct file *file, char const __user *buffer, size_t count, loff_t *f_pos){  
    char data[64];
    int i;
    int fib[2048];

    if(copy_from_user(data, buffer, count)) 
        return -EINVAL;
    
    int digits = count - 1;

    int number = 0;

    for(i = digits - 1; i >= 0; i--)
        number += (data[i] - '0')*pow(10, digits - i - 1);

    if(number <= 0)
        return 0;

    fib[0] = fib[1] = 1;

    for(i = 2; i <= number; i++){
        fib[i] = fib[i-1] + fib[i-2];
    }
    
    pr_info("Fibonacci: %i", fib[number-1]);

    int pos = 0;
    char tmp[32];
    digits = 1;
    dev_names[1].data_size = 0;

    for(i = 0; i < number; i++){
        sprintf(tmp, "%d", fib[i]);
        digits = count_diviter(fib[i]);
        memcpy(dev_names[1].data + pos, tmp, digits);
        pos += digits + 1;
        dev_names[1].data[pos] = ' ';
        pos += 1;
        dev_names[1].data_size = pos;
    }

    dev_names[1].data[pos] = 0;

    return count;
}

/********************* SAVE *************************/
static ssize_t save_read(struct file *file, char __user *buffer, size_t count, loff_t *f_pos){

    if(*f_pos >= dev_names[2].data_size)
        return 0;

    if(*f_pos + count > dev_names[2].data_size)
        count = dev_names[2].data_size - *f_pos;

    if(copy_to_user(buffer, dev_names[2].data, count))
        return -EFAULT; 

    *f_pos += count;

    return count;
}

static ssize_t save_write(struct file *file, char const __user *buffer, size_t count, loff_t *f_pos){
    
    if(copy_from_user(dev_names[2].data, buffer, count))
        return -EINVAL;

    dev_names[2].data[count] = 0;
    dev_names[2].data_size = count;

    return count;
}

/* Definition of the file_operations table for each device. */

static const struct file_operations led_fops = {
    .owner = THIS_MODULE,
    .open = open,
    .read = led_read,
    .write = led_write,
    .release = release
};

static const struct file_operations fibonacci_fops = {
    .owner = THIS_MODULE,
    .open = open,
    .read = fibonacci_read,
    .write = fibonacci_write,
    .release = release
};

static const struct file_operations save_fops = {
    .owner = THIS_MODULE,
    .open = open,
    .read = save_read,
    .write = save_write,
    .release = release
};

/* We define an struct that represent a device, including some space (64K) dedicated to store/recover information in any driver. */
static struct cdev DriverMiguelcdev[NUM_DEVICES];
static struct drivers {
    const char *name;
    const struct file_operations *fops;
} devlist[NUM_DEVICES] = {
    [0] = {"led", &led_fops},
    [1] = {"fibonacci", &fibonacci_fops},
    [2] = {"save", &save_fops}
};

/********************** Initialization of the controller *******************************/

static int driver_dev_uevent(struct device *dev, struct kobj_uevent_env *env){
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int __init init_driver(void){
    int n_device = 0;
    dev_t id_device;

    /* Get major number */
    if(alloc_chrdev_region(&major_minor, 0, NUM_DEVICES, DRIVER_CLASS) < 0){
        pr_err("Major number assingment failed.");

        goto error;
    }

    /* Check that our driver has been assigned a major number */
    pr_info("%s driver assigned %d major number\n", DRIVER_NAME, MAJOR(major_minor));

    /* Create the class */
    if((driverClass = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL){
        pr_err("Class device registering failed.");
        goto error;
    } else 
        driverClass->dev_uevent = driver_dev_uevent;

    pr_info("/sys/class/%s class driver registered\n", DRIVER_CLASS);

    for(n_device = 0; n_device < NUM_DEVICES; n_device++){
        cdev_init(&(DriverMiguelcdev[n_device]), devlist[n_device].fops);

        id_device = MKDEV(MAJOR(major_minor), MINOR(major_minor) + n_device);
        if(cdev_add(&(DriverMiguelcdev[n_device]), id_device, 1) == -1){
            pr_err("Device node creation failed.");
            goto error;
        }

        if(device_create(driverClass, NULL, id_device, NULL, devlist[n_device].name) == NULL){
            pr_err("Device node creation failed.");
            goto error;
        }

        pr_info("Device node /dev/%s created", devlist[n_device].name);
    }

    pr_info("DriverMiguel initialized and loaded.");
    return 0;
    
    error:
        if(driverClass)
            class_destroy(driverClass);
        
        if(major_minor != -1)
            unregister_chrdev_region(major_minor, NUM_DEVICES);

        return -1;
}

static void __exit exit_driver(void){
    int n_device;

    for(n_device = 0; n_device < NUM_DEVICES; n_device++){
        device_destroy(driverClass, MKDEV(MAJOR(major_minor), MINOR(major_minor) + n_device));
        cdev_del(&DriverMiguelcdev[n_device]);
    }

    class_destroy(driverClass);

    unregister_chrdev_region(major_minor, NUM_DEVICES);

    pr_info("DriverMiguel unloaded\n");
}

/* Meta data */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Miguel Díaz");
MODULE_VERSION("0.1");
MODULE_DESCRIPTION("Driver module for Hardware Programming assingment");

module_init(init_driver)
module_exit(exit_driver)
