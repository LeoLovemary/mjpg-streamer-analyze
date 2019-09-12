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

#ifndef MJPG_STREAMER_H
#define MJPG_STREAMER_H
#define SOURCE_VERSION "2.0"      //当前版本定义

/* FIXME take a look to the output_http clients thread marked with fixme if you want to set more then 10 plugins */
/*翻译：如果您想设置超过10个插件，请查看用FIXME标记的output_http客户端线程*/
#define MAX_INPUT_PLUGINS 10      //最大输入插件数设置
#define MAX_OUTPUT_PLUGINS 10     //最大输出插件数设置 
#define MAX_PLUGIN_ARGUMENTS 32     //插件允许最大参数个数设置 

#include <linux/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include <pthread.h>             

#ifdef DEBUG
#define DBG(...) fprintf(stderr, " DBG(%s, %s(), %d): ", __FILE__, __FUNCTION__, __LINE__); fprintf(stderr, __VA_ARGS__)
#else
#define DBG(...)
#endif

#define LOG(...) { char _bf[1024] = {0}; snprintf(_bf, sizeof(_bf)-1, __VA_ARGS__); fprintf(stderr, "%s", _bf); syslog(LOG_INFO, "%s", _bf); }

#include "plugins/input.h"
#include "plugins/output.h"

/* global variables that are accessed by all plugins */
/*翻译：所有插件都可以访问的全局变量*/
typedef struct _globals globals;

/* an enum to identify the commands destination*/
/*翻译：用于标识命令目的地的枚举*/
typedef enum {
    Dest_Input = 0,
    Dest_Output = 1,
    Dest_Program = 2,
} command_dest;

/* commands which can be send to the input plugin */
/*翻译：可以发送到输入插件的命令*/
//typedef enum _cmd_group cmd_group;
/*翻译：定义枚举_cmd_group cmd_group*/
enum _cmd_group {
    IN_CMD_GENERIC =        0, // if you use non V4L2 input plugin you not need to deal the groups.  //翻译：如果使用非V4L2输入插件，则不需要处理组。
    IN_CMD_V4L2 =           1,
    IN_CMD_RESOLUTION =     2,
    IN_CMD_JPEG_QUALITY =   3,
    IN_CMD_PWC =            4,
};

typedef struct _control control;   //typedef 常给结构体类型命名
struct _control {
    struct v4l2_queryctrl ctrl;
    int value;
    struct v4l2_querymenu *menuitems;
    /*  In the case the control a V4L2 ctrl this variable will specify
        that the control is a V4L2_CTRL_CLASS_USER control or not.
        For non V4L2 control it is not acceptable, leave it 0.
    */
   /*
   翻译：在这种情况下，控件V4L2 ctrl this变量将指定该控件是否是V4L2_CTRL_CLASS_USER控件。对于非V4L2控件，这是不可接受的，让它为0。
   */
    int class_id;
    int group;
};

struct _globals {
    int stop;

    /* input plugin */
    /*翻译：输入插件*/
    input in[MAX_INPUT_PLUGINS];
    int incnt;

    /* output plugin */
    /*翻译：输出插件*/
    output out[MAX_OUTPUT_PLUGINS];
    int outcnt;

    /* pointer to control functions */
    /*翻译：指向控制函数的指针*/
    //int (*control)(int command, char *details);
};

#endif
