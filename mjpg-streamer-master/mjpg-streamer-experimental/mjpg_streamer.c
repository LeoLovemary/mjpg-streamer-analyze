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

#include <stdio.h>   //标准输入输出头文件(定义了三个变量类型、一些宏和各种函数来执行输入和输出。)
#include <stdlib.h>   //标准库头文件 (stdlib.h里面定义了C，C++语言的五种变量类型、一些宏和通用工具函数。)
#include <unistd.h>  //unistd.h是linux/unix的系统调用，包含了许多 U N I X系统服务的函数原型,例如 r e a d， w r i t e和getpid函数。
#include <string.h>  //string .h 头文件定义了一个变量类型、一个宏和各种操作字符数组的函数。
#include <sys/ioctl.h>   // ioctl是设备驱动程序中对设备的I/O通道进行管理的函数。所谓对I/O通道进行管理，就是对设备的一些特性进行控制，例如串口的传输波特率、马达的转速等等。
#include <errno.h>      //该头文件定义了通过错误码来回报错误资讯的宏。errno 宏定义为一个 int 型态的左值, 包含任何函式使用errno功能所产生的上一个错误码。
#include <signal.h>     //signal.h是C标准函数库中的信号处理部分， 定义了程序执行时如何处理不同的信号。信号用作进程间通信， 报告异常行为（如除零）、用户的一些按键组合（如同时按下Ctrl与C键，产生信号SIGINT）。
#include <sys/socket.h>     //socket编程涉及到头文件sys/socket.h 和sys/types.h
#include <arpa/inet.h>     //里面包含了一些网络编中需要的头文件,还有一些结构体 
#include <sys/types.h>      //socket编程涉及到头文件sys/socket.h 和sys/types.h
#include <sys/stat.h>       //stat函数可以返回一个结构，里面包括文件的全部属性(sys/stat.h头文件，轻松获取文件属性)
#include <getopt.h>         //getopt是一个专门设计来减轻命令行处理负担的库函数，它可以在全局结构中记录命令参数，以便随后随时在整个程序中使用，即getopt被用来解析命令行选项参数，就不用自己写代码处理argv了。其中比较重要的函数是getopt()和getopt_long()。
#include <pthread.h>        //Linux系统下的多线程遵循POSIX线程接口，称为pthread。编写Linux下的多线程程序，需要使用头文件pthread.h，连接时需要使用库libpthread.a。
#include <dlfcn.h>          //dlfcn.h是一个头文件，调用动态链接库用的
#include <fcntl.h>          //fcntl.h，是unix标准中通用的头文件，其中包含的相关函数有 open，fcntl，shutdown，unlink，fclose等！
#include <syslog.h>         //Linux C中提供一套系统日记写入接口，包括三个函数：openlog，syslog和closelog。
#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>    //V4L2 的相关定义包含在头文件

#include "utils.h"
#include "mjpg_streamer.h"

/* globals */
static globals global;     //创建全局结构体变量

/***********************************************************************************************************************
Description.: Display a help message  //描述。:显示帮助信息
Input Value.: argv[0] is the program name and the parameter progname  //输入值。: argv[0]是程序名和参数progname
Return Value: -         //返回值:-
***********************************************************************************************************************/

static void help(char *progname)
{
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Usage: %s\n" \
            "  -i | --input \"<input-plugin.so> [parameters]\"\n" \
            "  -o | --output \"<output-plugin.so> [parameters]\"\n" \
            " [-h | --help ]........: display this help\n" \
            " [-v | --version ].....: display version information\n" \
            " [-b | --background]...: fork to the background, daemon mode\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example #1:\n" \
            " To open an UVC webcam \"/dev/video1\" and stream it via HTTP:\n" \
            "  %s -i \"input_uvc.so -d /dev/video1\" -o \"output_http.so\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example #2:\n" \
            " To open an UVC webcam and stream via HTTP port 8090:\n" \
            "  %s -i \"input_uvc.so\" -o \"output_http.so -p 8090\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "Example #3:\n" \
            " To get help for a certain input plugin:\n" \
            "  %s -i \"input_uvc.so --help\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
    fprintf(stderr, "In case the modules (=plugins) can not be found:\n" \
            " * Set the default search path for the modules with:\n" \
            "   export LD_LIBRARY_PATH=/path/to/plugins,\n" \
            " * or put the plugins into the \"/lib/\" or \"/usr/lib\" folder,\n" \
            " * or instead of just providing the plugin file name, use a complete\n" \
            "   path and filename:\n" \
            "   %s -i \"/path/to/modules/input_uvc.so\"\n", progname);
    fprintf(stderr, "-----------------------------------------------------------------------\n");
}

/**************************************************************************************************************************************
Description.: pressing CTRL+C sends signals to this process instead of just
              killing it plugins can tidily shutdown and free allocated
              resources. The function prototype is defined by the system,
              because it is a callback function.
 //该函数用于结束程序 （释放资源。）           
 //描述。:按CTRL+C向这个进程发送信号，而不仅仅是直接杀死插件，而可以整齐地关闭和释放分配的资源。函数原型由系统定义，因为它是一个回调函数。       

Input Value.: sig tells us which signal was received   //输入值。: sig告诉我们收到了哪个信号
Return Value: -
**************************************************************************************************************************************/
static void signal_handler(int sig)
{
    int i;

    /* signal "stop" to threads */
    /*向线程发出“停止”信号*/
    LOG("setting signal to stop\n");
    global.stop = 1;
    usleep(1000 * 1000);  //1s延迟

    /* clean up threads */
    /*清理线程*/
    LOG("force cancellation of threads and cleanup resources\n");
    for(i = 0; i < global.incnt; i++) {
        global.in[i].stop(i);
        /*for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            if (global.in[i].param.argv[j] != NULL) {
                free(global.in[i].param.argv[j]);
            }
        }*/
    }

    for(i = 0; i < global.outcnt; i++) {
        global.out[i].stop(global.out[i].param.id);
        pthread_cond_destroy(&global.in[i].db_update);
        pthread_mutex_destroy(&global.in[i].db);
        /*for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            if (global.out[i].param.argv[j] != NULL)
                free(global.out[i].param.argv[j]);
        }*/
    }
    usleep(1000 * 1000); //1s延迟

    /* close handles of input plugins */
    /*关闭输入插件的句柄*/
    for(i = 0; i < global.incnt; i++) {
        dlclose(global.in[i].handle);
    }

    for(i = 0; i < global.outcnt; i++) {
        int j, skip = 0;
        DBG("about to decrement usage counter for handle of %s, id #%02d, handle: %p\n", \
            global.out[i].plugin, global.out[i].param.id, global.out[i].handle);

        for(j=i+1; j<global.outcnt; j++) {
          if ( global.out[i].handle == global.out[j].handle ) {
            DBG("handles are pointing to the same destination (%p == %p)\n", global.out[i].handle, global.out[j].handle);
            skip = 1;
          }
        }
        if ( skip ) {
          continue;
        }

        DBG("closing handle %p\n", global.out[i].handle);

        dlclose(global.out[i].handle);
    }
    DBG("all plugin handles closed\n");

    LOG("done\n");

    closelog();
    exit(0);
    return;
}

/******************************************************************************
Description.:  split_parameters用于提取命令行参数
Input Value.: 
    *parameter_string->待处理参数字符串
    *argc->参数个数
    **argv->参数数组
Return Value:
*****************************************************************************/
static int split_parameters(char *parameter_string, int *argc, char **argv)
{
    int count = 1;
    argv[0] = NULL; // the plugin may set it to 'INPUT_PLUGIN_NAME'
    if(parameter_string != NULL && strlen(parameter_string) != 0) {
        char *arg = NULL, *saveptr = NULL, *token = NULL;

        arg = strdup(parameter_string); //strdup返回值：返回一个指针,指向为复制字符串分配的空间;如果分配空间失败,则返回NULL值。
        //判断是否存在参数
        if(strchr(arg, ' ') != NULL) {
            //strtok_r()函数用于分割字符串。strtok_r是linux平台下的strtok函数的线程安全版。
            token = strtok_r(arg, " ", &saveptr); //第一次调用strtok_r时，str参数必须指向待提取的字符串，saveptr参数的值可以忽略。
            if(token != NULL) {
                argv[count] = strdup(token);
                count++;
                //连续调用时，str赋值为NULL，saveptr为上次调用后返回的值，不要修改。
                //提取所有参数
                while((token = strtok_r(NULL, " ", &saveptr)) != NULL) {
                    argv[count] = strdup(token);
                    count++;
                    //如果参数个数超过限定数则报错
                    if(count >= MAX_PLUGIN_ARGUMENTS) {
                        IPRINT("ERROR: too many arguments to input plugin\n");
                        return 0;
                    }
                }
            }
        }
        free(arg); //释放内存
    }
    *argc = count;
    return 1;
}

/******************************************************************************
Description.: 主程序
Input Value.: int argc, char *argv[]
Return Value: return -
******************************************************************************/
int main(int argc, char *argv[])
{
    //char *input  = "input_uvc.so --resolution 640x480 --fps 5 --device /dev/video0";
    char *input[MAX_INPUT_PLUGINS];    //指针数组
    char *output[MAX_OUTPUT_PLUGINS];
    int daemon = 0, i, j;
    size_t tmp = 0;

    output[0] = "output_http.so --port 8080";   //创建一个输出插件
    global.outcnt = 0;  //输出插件数置0
    global.incnt = 0;   //输入插件数置0

    /* parameter parsing */
    /*参数传递*/
    while(1) {
        int c = 0;
        /*创建option结构的数组*/
        static struct option long_options[] = {
            {"help", no_argument, NULL, 'h'}, //无需参数，指定getopt_long返回值‘h’
            {"input", required_argument, NULL, 'i'},  //需要参数，指定getopt_long返回值‘i’
            {"output", required_argument, NULL, 'o'},
            {"version", no_argument, NULL, 'v'},
            {"background", no_argument, NULL, 'b'},
            {NULL, 0, NULL, 0}
        };

        c = getopt_long(argc, argv, "hi:o:vb", long_options, NULL);  //获取参数（自动指向下一个参数）

        /* no more options to parse */
        /*c == -1 表示无参数或解析错误，则结束while循环*/
        if(c == -1) break;  
        /*根据参数分类处理*/
        switch(c) {
        case 'i':  //添加输入插件
            input[global.incnt++] = strdup(optarg);   
            break;

        case 'o': //添加输出插件
            output[global.outcnt++] = strdup(optarg);
            break;

        case 'v'://打印版本号
            printf("MJPG Streamer Version: %s\n",
#ifdef GIT_HASH
            GIT_HASH
#else
            SOURCE_VERSION
#endif
            );
            return 0;
            break;

        case 'b': 
            daemon = 1;  //打开创建守护进程标志
            break;

        case 'h': /* fall through */
        default:
            help(argv[0]); //调用帮助程序
            exit(EXIT_FAILURE);   //结束程序
        }
    }

    openlog("MJPG-streamer ", LOG_PID | LOG_CONS, LOG_USER);
    //openlog("MJPG-streamer ", LOG_PID|LOG_CONS|LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "starting application");

    /* fork to the background */
    /*判断是否创建守护进程*/
    if(daemon) {
        LOG("enabling daemon mode");
        daemon_mode(); //进入守护进程模式
    }


    /*
    signal（参数1，参数2）；
    参数1：我们要进行处理的信号。系统的信号我们可以再终端键入 kill -l查看(共64个)。其实这些信号时系统定义的宏。
    参数2：我们处理的方式（是系统默认还是忽略还是捕获）。
    一般有3中方式进行操作。
    */

   /*signal(SIGPIPE, SIG_IGN)函数说明**************************************************************************
    /* ignore SIGPIPE (send by OS if transmitting to closed TCP sockets)
   忽略SIGPIPE(如果传输到关闭的TCP套接字，则由OS发送)
   连接建立，若某一端关闭连接，而另一端仍然向它写数据，第一次写数据后会收到RST响应，此后再写数据，
   内核将向进程发出SIGPIPE信号，通知进程此连接已经断开。而SIGPIPE信号的默认处理是终止程序,所以下面忽略此信号。
   */
    signal(SIGPIPE, SIG_IGN); //忽略SIGPIPE信号,避免程序异常终止

    /* register signal handler for <CTRL>+C in order to clean up */
    /*为<CTRL>+C注册信号处理程序，以便进行清理*/
    /*SIGINT：<CTRL>+C信号   signal_handler:回调函数*/
    if(signal(SIGINT, signal_handler) == SIG_ERR) {
        LOG("could not register signal handler\n");
        closelog();
        exit(EXIT_FAILURE);
    }

    /*
     * messages like the following will only be visible on your terminal
     * if not running in daemon mode
     */
#ifdef GIT_HASH
    LOG("MJPG Streamer Version: git rev: %s\n", GIT_HASH);
#else
    LOG("MJPG Streamer Version.: %s\n", SOURCE_VERSION);
#endif

    /* check if at least one output plugin was selected */
    /*检查是否至少选择了一个输出插件*/
    if(global.outcnt == 0) {
        /* 没有?然后使用默认插件*/
        /**/
        global.outcnt = 1;
    }

    /* open input plugin */
    /*打开输入插件*/
    for(i = 0; i < global.incnt; i++) {
        /* this mutex and the conditional variable are used to synchronize access to the global picture buffer */
        /*这个互斥量和条件变量用于同步对全局图片缓冲区的访问*/
        /*   &global.in[i].db  取得返回的当前输入插件对应的互斥量（初始化并且获取输入插件对应互斥量）*/
        if(pthread_mutex_init(&global.in[i].db, NULL) != 0) {
            LOG("could not initialize mutex variable\n");
            closelog();
            exit(EXIT_FAILURE);
        }
        /*函数pthread_cond_init（）被用来初始化一个条件变量。*/
        if(pthread_cond_init(&global.in[i].db_update, NULL) != 0) {
            LOG("could not initialize condition variable\n");
            closelog();
            exit(EXIT_FAILURE);
        }
      
        tmp = (size_t)(strchr(input[i], ' ') - input[i]);
        global.in[i].stop      = 0;
        global.in[i].context   = NULL;
        global.in[i].buf       = NULL;
        global.in[i].size      = 0;
        global.in[i].plugin = (tmp > 0) ? strndup(input[i], tmp) : strdup(input[i]);
        global.in[i].handle = dlopen(global.in[i].plugin, RTLD_LAZY);
        if(!global.in[i].handle) {
            LOG("ERROR: could not find input plugin\n");
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            closelog();
            exit(EXIT_FAILURE);
        }
        global.in[i].init = dlsym(global.in[i].handle, "input_init");
        if(global.in[i].init == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.in[i].stop = dlsym(global.in[i].handle, "input_stop");
        if(global.in[i].stop == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.in[i].run = dlsym(global.in[i].handle, "input_run");
        if(global.in[i].run == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        /* try to find optional command */
        global.in[i].cmd = dlsym(global.in[i].handle, "input_cmd");

        global.in[i].param.parameters = strchr(input[i], ' ');

        for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            global.in[i].param.argv[j] = NULL;
        }

        split_parameters(global.in[i].param.parameters, &global.in[i].param.argc, global.in[i].param.argv);
        global.in[i].param.global = &global;
        global.in[i].param.id = i;

        if(global.in[i].init(&global.in[i].param, i)) {
            LOG("input_init() return value signals to exit\n");
            closelog();
            exit(0);
        }
    }

    /* open output plugin */
    for(i = 0; i < global.outcnt; i++) {
        tmp = (size_t)(strchr(output[i], ' ') - output[i]);
        global.out[i].plugin = (tmp > 0) ? strndup(output[i], tmp) : strdup(output[i]);
        global.out[i].handle = dlopen(global.out[i].plugin, RTLD_LAZY);
        if(!global.out[i].handle) {
            LOG("ERROR: could not find output plugin %s\n", global.out[i].plugin);
            LOG("       Perhaps you want to adjust the search path with:\n");
            LOG("       # export LD_LIBRARY_PATH=/path/to/plugin/folder\n");
            LOG("       dlopen: %s\n", dlerror());
            closelog();
            exit(EXIT_FAILURE);
        }
        global.out[i].init = dlsym(global.out[i].handle, "output_init");
        if(global.out[i].init == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.out[i].stop = dlsym(global.out[i].handle, "output_stop");
        if(global.out[i].stop == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }
        global.out[i].run = dlsym(global.out[i].handle, "output_run");
        if(global.out[i].run == NULL) {
            LOG("%s\n", dlerror());
            exit(EXIT_FAILURE);
        }

        /* try to find optional command */
        global.out[i].cmd = dlsym(global.out[i].handle, "output_cmd");

        global.out[i].param.parameters = strchr(output[i], ' ');

        for (j = 0; j<MAX_PLUGIN_ARGUMENTS; j++) {
            global.out[i].param.argv[j] = NULL;
        }
        split_parameters(global.out[i].param.parameters, &global.out[i].param.argc, global.out[i].param.argv);

        global.out[i].param.global = &global;
        global.out[i].param.id = i;
        if(global.out[i].init(&global.out[i].param, i)) {
            LOG("output_init() return value signals to exit\n");
            closelog();
            exit(EXIT_FAILURE);
        }
    }

    /* start to read the input, push pictures into global buffer */
    DBG("starting %d input plugin\n", global.incnt);
    for(i = 0; i < global.incnt; i++) {
        syslog(LOG_INFO, "starting input plugin %s", global.in[i].plugin);
        if(global.in[i].run(i)) {
            LOG("can not run input plugin %d: %s\n", i, global.in[i].plugin);
            closelog();
            return 1;
        }
    }

    DBG("starting %d output plugin(s)\n", global.outcnt);
    for(i = 0; i < global.outcnt; i++) {
        syslog(LOG_INFO, "starting output plugin: %s (ID: %02d)", global.out[i].plugin, global.out[i].param.id);
        global.out[i].run(global.out[i].param.id);
    }

    /* wait for signals */
    pause();

    return 0;
}
