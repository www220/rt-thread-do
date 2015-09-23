/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Change Logs:
 * Date           Author		Notes
 * 2011-01-13     weety		first version
 */

/**
 * @addtogroup at91sam9260
 */
/*@{*/

#include <board.h>
#include <rtthread.h>

#define LED_ERR_PORT		PIN_ERR
#define LED_ERR_PIN		
#define LED_RUN_PORT	PIN_RUN
#define LED_RUN_PIN		

#define WDT_PORT		PIN_WDT
#define WDT_PIN			

#ifdef RT_USING_DFS
#include <dfs_init.h>
#include <dfs_fs.h>
#ifdef RT_USING_DFS_ELMFAT
#include <dfs_elm.h>
#endif
#ifdef RT_USING_DFS_YAFFS2
extern void dfs_yaffs_init(void);
#endif
#ifdef RT_USING_DFS_DEVFS
#include <devfs.h>
#endif
#ifdef RT_USING_DFS_ROMFS
#include <dfs_romfs.h>
#endif
#ifdef RT_USING_SDIO
extern void tf_init(void);
#endif
#ifdef RT_USING_MTD_NAND
extern void nand_init(void);
#endif
#endif

#ifdef RT_USING_RTC
extern void list_date(void);
#endif

#ifdef RT_USING_LIBC
extern void libc_system_init(void);
#endif

#ifdef RT_USING_LWIP
#include <lwip/sys.h>
#include <lwip/api.h>
#include <netif/ethernetif.h>
extern void rt_hw_eth_init(void);
extern int eth_system_device_init(void);
extern void lwip_sys_init(void);
#endif

#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef RT_USING_SQLITE3
#include <sqlite3.h>
#endif

#ifdef RT_USING_RTGUI
extern int rtgui_system_server_init(void);
extern int lcd_init(void);
extern int touch_init(void);
#endif

#ifdef FINSH_USING_MSH
extern int cmd_sh(int argc, char** argv);
extern int cmd_beep(int argc, char** argv);
#endif

volatile int eth_wtdog = 0;
volatile int eth_linkstatus = 0;
volatile int wtdog_count = 0;
volatile int sys_stauts = -1;

unsigned char NET_MAC[6] = {0};
extern void cpu_usage_idle_hook(void);

#ifdef _MSC_VER
#define GPIO_ResetBits(x,y)
#define GPIO_SetBits(x,y)
#else
#define GPIO_ResetBits(x,y) pin_gpio_set(x,0)
#define GPIO_SetBits(x,y) pin_gpio_set(x,1)
#endif

ALIGN(RT_ALIGN_SIZE)
static char thread_wtdog_stack[2048];
struct rt_thread thread_wtdog;
static void rt_thread_entry_wtdog(void* parameter)
{
    int runcount = 0;
    int ledcount = 0;
    float fdata = 0;
    while (1)
    {
        //在看住系统运行状态的同时看住网络是否正常
        if ((wtdog_count < 15) && (!eth_linkstatus || eth_wtdog < 15))
        {
            //wdt
            rt_thread_delay(500);
            GPIO_ResetBits(WDT_PORT, WDT_PIN);
            rt_thread_delay(500);
            GPIO_SetBits(WDT_PORT, WDT_PIN);
            wtdog_count++;
            //status
            if (sys_stauts < 0)
            {
                //run
                if (ledcount == 0)
                {
                    GPIO_SetBits(LED_RUN_PORT, LED_RUN_PIN);
                    ledcount = 1;
                }
                else
                {
                    GPIO_ResetBits(LED_RUN_PORT, LED_RUN_PIN);
                    ledcount = 0;
                }
                GPIO_SetBits(LED_ERR_PORT, LED_ERR_PIN);
                runcount = 0;
            }
            else if(sys_stauts == 0)
            {
                //err
                GPIO_ResetBits(LED_ERR_PORT, LED_ERR_PIN);
                GPIO_SetBits(LED_RUN_PORT, LED_RUN_PIN);
                runcount = 0;
            }
            else
            {
                if (++runcount >= sys_stauts)
                {
                    //err
                    if (ledcount == 0)
                    {
                        GPIO_SetBits(LED_ERR_PORT, LED_ERR_PIN);
                        ledcount = 1;
                    }
                    else
                    {
                        GPIO_ResetBits(LED_ERR_PORT, LED_ERR_PIN);
                        ledcount = 0;                    
                    }
                    GPIO_SetBits(LED_RUN_PORT, LED_RUN_PIN);
                    runcount = 0;
                }
            }
        }
        else
        {
            //err
            GPIO_SetBits(LED_RUN_PORT, LED_RUN_PIN);
            GPIO_ResetBits(LED_ERR_PORT, LED_ERR_PIN);
            rt_thread_delay(1000);
        }
        //这个代码没有任何的意义只是为了能在代码中使用float而已
        if (fdata++ > 10e20)
        {
            fdata = 0;
            rt_kprintf("wtdog_fdata\n");
        }
    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread_main_stack[8192];
struct rt_thread thread_main;
static void rt_thread_entry_main(void* parameter)
{
    /* initialize rtc */
#ifdef RT_USING_RTC
    rt_hw_rtc_init();
#endif

    /* initialize the device file system */
#ifdef RT_USING_DFS
    dfs_init();

#ifdef RT_USING_DFS_ROMFS
    dfs_romfs_init();
#endif

#ifdef RT_USING_DFS_DEVFS
    devfs_init();
#endif

#if defined(RT_USING_MTD_NAND) && defined(RT_USING_DFS_YAFFS2)
    nand_init();
    dfs_yaffs_init();
#endif

#ifdef RT_USING_DFS_ROMFS
    dfs_mount(RT_NULL, "/", "rom", 0, &romfs_root);
    rt_kprintf("File System initialized!\n");
#endif
#ifdef RT_USING_DFS_DEVFS
    dfs_mount(RT_NULL, "/dev", "devfs", 0, 0);
    rt_kprintf("Mount /dev ok!\n");
#endif
#if defined(RT_USING_MTD_NAND) && defined(RT_USING_DFS_YAFFS2)
    if (dfs_mount("nand0", "/mnt", "yaffs2", 0, 0) == 0)
        rt_kprintf("Mount /mnt ok!\n");
    else
        rt_kprintf("Mount /mnt failed!\n");
#endif

#if defined(RT_USING_SDIO) && defined(RT_USING_DFS_ELMFAT)
    tf_init();
    elm_init();
#endif
#if defined(RT_USING_SDIO) && defined(RT_USING_DFS_ELMFAT)
    if (dfs_mount("mmc", "/mmc", "elm", 0, 0) == 0)
        rt_kprintf("Mount /mmc ok!\n");
    else
        rt_kprintf("Mount /mmc failed!\n");
#endif

#endif

#ifdef RT_USING_LIBC
    libc_system_init();
#endif

    /* LwIP Initialization */
#ifdef RT_USING_LWIP
    eth_system_device_init();
    rt_hw_eth_init();
    lwip_sys_init();
    rt_kprintf("TCP/IP initialized!\n");
#endif

    inittmppath();
    rt_thread_idle_sethook(cpu_usage_idle_hook);
#ifdef RT_USING_SQLITE3
    sqlite3_initialize();
    rt_kprintf("Sqlite3 initialized!\n");
#endif

#ifdef RT_USING_RTGUI
    lcd_init();
    touch_init();
    rtgui_system_server_init();
    rt_kprintf("RtGUI initialized!\n");
#endif

    /* list date */
#ifdef RT_USING_RTC
    list_date();
#endif

    /* init finsh */
#ifdef RT_USING_FINSH
    finsh_system_init();
    finsh_set_device(FINSH_DEVICE_NAME);
   	rt_thread_delay(100);
#endif

#if defined(FINSH_USING_MSH)
    {
    	char * cmd[] = {"sh", rttCfgFileDir "/auto.sh"};
    	char * beep[] = {"beep", "200"};
    	cmd_beep(0,NULL);
    	cmd_sh(2, cmd);
    	cmd_beep(2,beep);
    }
#endif

    while (1)
        rt_thread_delay(1000);
}

int rt_application_init()
{
    //------- dog thread
    rt_thread_init(&thread_wtdog,
                   "wtdog",
                   rt_thread_entry_wtdog,
                   RT_NULL,
                   &thread_wtdog_stack[0],
                   sizeof(thread_wtdog_stack),2,10);
    rt_thread_startup(&thread_wtdog);
    //------- init main thread
    rt_thread_init(&thread_main,
                   "main",
                   rt_thread_entry_main,
                   RT_NULL,
                   &thread_main_stack[0],
                   sizeof(thread_main_stack),10,10);
    rt_thread_startup(&thread_main);

    return 0;
}

/*@}*/
