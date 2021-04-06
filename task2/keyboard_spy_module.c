/**
 * @file keyboard_spy_module.c
 *
 * @brief PS/2 keyboard interrupt handler
 * @author Bobylev Igor, 
 * Contact: igor.v.bobylev@gmail.com
 *
 */


#include <asm/atomic.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

MODULE_AUTHOR("Bobylev Igor");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("PS/2 keyboard interrupt handler");

#define KEYBOARD_IRQ 1
#define TIMEOUT 60000
#define MODULE_PREFIX "<KEBOARD HANDLER> "

static unsigned long long g_dev_id;

static atomic_t g_atomic_count;

static struct timer_list g_timer;

static irqreturn_t handler(int irq, void* dev_id);
void timer_handler(struct timer_list* t);

static int __init init_irq_module(void)
{
    int res = 0;
    
    atomic_set(&g_atomic_count, 0);

    res = request_irq(KEYBOARD_IRQ, handler, IRQF_SHARED, "keyboard handler", &g_dev_id);
    if (0 != res)
    {
        printk(MODULE_PREFIX "Failed request_irq with responce %d\n", -res);
        return -EFAULT;
    }

    timer_setup(&g_timer, timer_handler, 0);

    if (mod_timer(&g_timer, jiffies + msecs_to_jiffies(TIMEOUT)))
    {
        printk(MODULE_PREFIX "Failed mod_timer\n");
        free_irq(KEYBOARD_IRQ, &g_dev_id);
        return -EFAULT;
    }

    printk(MODULE_PREFIX "Module loaded\n");
    return 0;
}

static void __exit exit_irq_module(void)
{
    free_irq(KEYBOARD_IRQ, &g_dev_id);
    del_timer(&g_timer);
    printk(MODULE_PREFIX "Module unloaded\n");
}

static irqreturn_t handler(int irq, void* dev_id)
{
    uint8_t scancode = 0;

    if (KEYBOARD_IRQ != irq)
    {
        return IRQ_NONE;
    }

    scancode = inb(0x60);

    if (0 == scancode)
    {
        printk(MODULE_PREFIX "Error code\n");
        return IRQ_NONE;
    }
    if (scancode >= 0x80)
    {
        return IRQ_NONE;
    }
    
    atomic_add(1, &g_atomic_count);

    return IRQ_HANDLED;
}

void timer_handler(struct timer_list* t)
{
    const int val = atomic_xchg(&g_atomic_count, 0);
    printk(MODULE_PREFIX "number of keystrokes per minute: %d\n", val);
    mod_timer(&g_timer, jiffies + msecs_to_jiffies(TIMEOUT));
}

module_init(init_irq_module);
module_exit(exit_irq_module);
