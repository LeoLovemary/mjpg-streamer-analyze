/*******************************************************************************
#                                                                              #
#      MJPG-streamer allows to stream JPG frames from an input-plugin          #
#      to several output plugins                                               #
# 【翻译：MJPG-streamer允许将JPG帧从输入插件流到多个输出插件】                     #      
#                                                                              #
#      Copyright (C) 2007 Tom Stöveken                                         #
# 【翻译：版权所有(C) 2007汤姆斯托维肯】                                          #           
#                                                                              #
# This program is free software; you can redistribute it and/or modify         #
# it under the terms of the GNU General Public License as published by         #
# the Free Software Foundation; version 2 of the License.                      #
# 【翻译：这个程序是自由软件;您可以根据自由软件基金会发布的GNU通用公共               #
#  许可证条款重新发布和/或修改它;许可的版本2。】                                   # 
#                                                                              #
# This program is distributed in the hope that it will be useful,              #
# but WITHOUT ANY WARRANTY; without even the implied warranty of               #
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                #
# GNU General Public License for more details.                                 #
#【翻译：本程序的发布是希望它将是有用的，但没有任何保证;甚至没有隐含的适             #   
#销性或适合某一特定用途的保证。有关更多细节，请参阅GNU通用公共许可证。】             #
#                                                                              #         
# You should have received a copy of the GNU General Public License            #
# along with this program; if not, write to the Free Software                  #
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA    #
#  【翻译：你应该已经收到一份GNU通用公共许可证的副本连同这个程序;如果没              #
#有，写信给自由软件基金会，Inc.， 59 Temple Place, Suite 330, Boston,             #
#MA 02111-1307 USA】                                                            #
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <string.h>
#include <fcntl.h>
#include <wait.h>
#include <time.h>
#include <limits.h>  //limits.h专门用于检测整型数据数据类型的表达值范围。
#include <linux/stat.h>
#include <sys/stat.h>

#include "utils.h"

/******************************************************************************
Description.: 创建守护进程  【让程序后台运行】
Input Value.:
Return Value:
******************************************************************************/
void daemon_mode(void)
{
    int fr = 0;

    fr = fork();   //利用fork()创建进程//fork（）函数通过系统调用创建一个与原来进程几乎完全相同的进程,相当于克隆一个进程 
    if(fr < 0) {
        fprintf(stderr, "fork() failed\n");
        exit(1);
    }

    if(fr > 0) {
        exit(0);   //父进程退出，使子进程成为孤儿进程
    }
/*当进程是会话的领头进程时setsid()调用失败并返回（-1）。setsid()调用成功后，返回新的会话的ID，调用setsid函数的进程成为新的会话的
领头进程，并与其父进程的会话组和进程组脱离。由于会话对控制终端的独占性，进程同时与控制终端脱离。
*/
//开启新的会话
    if(setsid() < 0) {
        fprintf(stderr, "setsid() failed\n");
        exit(1);
    }
/*在调用了fork函数时，子进程全盘拷贝了父进程的会话期、进程组、控制终端等。虽然父进程退出了，但会话期、进程组、控制终端等并没有改变。
  因此，这还不是真正意义上的独立开来，而setsid函数能够使进程完全独立出来，从而摆脱其他进程的控制。
*/
    fr = fork();
    if(fr < 0) {
        fprintf(stderr, "fork() failed\n");
        exit(1);
    }
    if(fr > 0) {
        fprintf(stderr, "forked to background (%d)\n", fr);
        exit(0);
    }
/*
就是设置允许当前进程创建文件或者目录最大可操作的权限，比如这里设置为0，
它的意思就是0取反再创建文件时权限相与，也就是：(~0) & mode 等于八进制的值0777 & mode了，
这样就是给后面的代码调用函数mkdir给出最大的权限，避免了创建目录或文件的权限不确定性。
*/
    umask(0);  //就是设置文件权限。把文件权限掩码设置为0，可以大大增强该守护进程的灵活性。

    fr = chdir("/");  //改变当前工作目录
    if(fr != 0) {
        fprintf(stderr, "chdir(/) failed\n");
        exit(0);
    }
/*
在上面的第三步之后，守护进程已经与所属的控制终端失去了联系。因此从终端输入的字符
不可能达到守护进程，守护进程中用常规方法（如printf）输出的字符也不可能在终端上显
示出来。所以，文件描述符为0、1和2 的3个文件（常说的输入、输出和报错）已经失去
了存在的价值，也应被关闭。
*/
    close(0);
    close(1);
    close(2);

    open("/dev/null", O_RDWR);  //创建空设备//写入/dev/null的东西会被系统丢掉 

  //描述符置0 
    fr = dup(0);
    fr = dup(0);
}


/*
 * Common webcam resolutions with information from
 * http://en.wikipedia.org/wiki/Graphics_display_resolution
 */
static const struct {
    const char *string;
    const int width, height;
} resolutions[] = {
    { "QQVGA", 160,  120  },
    { "QCIF",  176,  144  },
    { "CGA",   320,  200  },
    { "QVGA",  320,  240  },
    { "CIF",   352,  288  },
    { "PAL",   720,  576  },
    { "VGA",   640,  480  },
    { "SVGA",  800,  600  },
    { "XGA",   1024, 768  },
    { "HD",    1280, 720  },
    { "SXGA",  1280, 1024 },
    { "UXGA",  1600, 1200 },
    { "FHD",   1920, 1280 },
};

/******************************************************************************
Description.: convienence function for input plugins
Input Value.:
Return Value:
******************************************************************************/
void parse_resolution_opt(const char * optarg, int * width, int * height) {
    int i;

    /* try to find the resolution in lookup table "resolutions" */
    for(i = 0; i < LENGTH_OF(resolutions); i++) {
        if(strcmp(resolutions[i].string, optarg) == 0) {
            *width  = resolutions[i].width;
            *height = resolutions[i].height;
            return;
        }
    }
    
    /* parse value as decimal value */
    if (sscanf(optarg, "%dx%d", width, height) != 2) {
        fprintf(stderr, "Invalid height/width '%s' specified!\n", optarg);
        exit(EXIT_FAILURE);
    }
}

void resolutions_help(const char * padding) {
    int i;
    for(i = 0; i < LENGTH_OF(resolutions); i++) {
        fprintf(stderr, "%s ", resolutions[i].string);
        if((i + 1) % 6 == 0)
            fprintf(stderr, "\n%s", padding);
    }
    fprintf(stderr, "\n%sor a custom value like the following" \
    "\n%sexample: 640x480\n", padding, padding);
}
