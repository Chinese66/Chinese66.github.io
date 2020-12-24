/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 *
 * @file        main.c
 *
 * @brief       User application entry
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <board.h>

#include <shell.h>
#include <os_clock.h>

#include "st7789vw.h"
#include <vfs_fs.h>
#include <vfs_posix.h>
#include <oneos_config.h>

#define ardupilot_number (64800)//240*135*2
static uint8_t buffer [ardupilot_number];

static struct os_memheap system_heap;
int add_32k_sram()
{
#if defined(OS_USING_HEAP)
#if defined(OS_USING_MEM_HEAP) && defined(OS_USING_MEM_HEAP_AS_HEAP)
    os_memheap_init(&system_heap, "sram2", (void *)STM32_SRAM2_START, STM32_SRAM2_SIZE * 1024);
#endif
#endif
}
OS_DEVICE_INIT(add_32k_sram);


static void user_task(void *parameter)
{
    int i = 0;

//		while(1)
    {
        os_task_msleep(200);
    }

    if (vfs_mount("sd0", "/", "fat", 0, 0) == 0)
    {
        os_kprintf("Filesystem initialized!\n");
    }
    else
    {
        os_kprintf("Failed to initialize filesystem!\n");
    }


    int fd = -1;
    fd = open("/ardupilot_merge.bin", O_RDONLY);

    while(1)
    {
        os_task_msleep(1000);

        lcd_clear(BLACK);


        int length=-1;


        os_kprintf("fd:%d\n",fd);

        os_uint32_t tick=0,fps,last_j=0;
        float fps_f;
        os_uint32_t read_tick=0;
        os_uint32_t display_tick=0;
        char fps_buf[]="fps:12.3";
        char buf_path[]="/test/1234.bin";

        for (int j = 1; j < 1590; j+=1)  //由于屏幕刷新实在太慢，只好抽帧了。
        {

            lseek(fd,(ardupilot_number*j),0);
            read_tick = os_tick_get();
            length = read(fd, buffer,ardupilot_number);


            //文件偏移，以读取下一段数据，可读取显示合并前的文件
//			  int fd1;
//        read_tick = os_tick_get();
//			  sprintf(buf_path,"/test/%04d.bin",j);
//			  fd1 = open(buf_path, O_RDONLY);
//        length = read(fd1, buffer,ardupilot_number);
//			  close(fd1);

//						os_kprintf("len:%d\n",length);
            read_tick = os_tick_get()-read_tick;

            display_tick=os_tick_get();
            lcd_show_image(0, 60, 240,135,buffer);
            display_tick=os_tick_get()-display_tick;

            if(os_tick_get()-tick>=1000)//计算fps
            {
                fps=j-last_j;
                fps_f=(fps*1.0)/((os_tick_get()-tick)/1000.0);

                sprintf(fps_buf,"fps:%2.1f",fps_f);
                lcd_show_string(1,1,16,fps_buf);
                os_kprintf("%s\n",fps_buf);
                os_kprintf("%6d,read_tick:%d,display_tick:%d\n",os_tick_get(),read_tick,display_tick);
                tick=os_tick_get();
                last_j=j;
                os_task_mdelay(1);
            }

        }
    }

}


int main(void)
{
    os_task_t *task;

    task = os_task_create("user", user_task, NULL, 2048, 3, 5);
    OS_ASSERT(task);
    os_task_startup(task);

    return 0;
}
