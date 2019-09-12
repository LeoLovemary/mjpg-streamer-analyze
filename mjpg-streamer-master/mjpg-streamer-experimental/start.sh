#!/bin/sh

#*******************************************************************************
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
#*******************************************************************************/

## This example shows how to invoke mjpg-streamer from the command line

export LD_LIBRARY_PATH="$(pwd)"
#./mjpg_streamer -i "input_uvc.so --help"

./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www"
#./mjpg_streamer -i "./input_uvc.so -n -f 30 -r 1280x960"  -o "./output_http.so -w ./www" 
#./mjpg_streamer -i "./input_uvc.so -n -f 30 -r 640x480 -d /dev/video0"  -o "./output_http.so -w ./www" &
#./mjpg_streamer -i "./input_uvc.so -d /dev/video0" -i "./input_uvc.so -d /dev/video1" -o "./output_http.so -w ./www"
#valgrind ./mjpg_streamer -i "./input_uvc.so" -o "./output_http.so -w ./www"

#./mjpg_streamer -i "./input_uvc.so -n -f 30 -r 1280x960" -o "./output_udpserver.so --port 2001"
#./mjpg_streamer -i "./input_uvc.so -n -f 30 -r 640x480" -o "./output_udpserver.so --address epgm://eth0;239.1.1.1:2001"
#./mjpg_streamer -i "./input_uvc.so -n -f 30 -r 640x480 -d /dev/video0" -o "./output_zmqserver.so --address tcp://*:2001 --buffer_size 2"
## pwd echos the current path you are working at,
## the backticks open a subshell to execute the command pwd first
## the exported variable name configures ldopen() to search a certain
## folder for *.so modules
#export LD_LIBRARY_PATH=`pwd`

## this is the minimum command line to start mjpg-streamer with webpages
## for the input-plugin default parameters are used
#./mjpg_streamer -o "output_http.so -w `pwd`/www"

## to query help for the core:
# ./mjpg_streamer --help

## to query help for the input-plugin "input_uvc.so":
# ./mjpg_streamer --input "input_uvc.so --help"

## to query help for the output-plugin "output_file.so":
# ./mjpg_streamer --output "output_file.so --help"

## to query help for the output-plugin "output_http.so":
# ./mjpg_streamer --output "output_http.so --help"

## to specify a certain device, framerage and resolution for the input plugin:
# ./mjpg_streamer -i "input_uvc.so -d /dev/video2 -r 320x240 -f 10"

## to start both, the http-output-plugin and write to files every 15 second:
# mkdir pics
# ./mjpg_streamer -o "output_http.so -w `pwd`/www" -o "output_file.so -f pics -d 15000"

## to protect the webserver with a username and password (!! can easily get sniffed and decoded, it is just base64 encoded !!)
# ./mjpg-streamer -o "output_http.so -w ./www -c UsErNaMe:SeCrEt"

## If you want to track down errors, use this simple testpicture plugin as input source.
## to use the testpicture input plugin instead of a webcam or folder:
#./mjpg_streamer -i "input_testpicture.so -r 320x240 -d 500" -o "output_http.so -w www"

## The input_file.so plugin watches a folder for new files, it does not matter where
## the JPEG files orginate from. For instance it is possible to grab the desktop and 
## store the files to a folder:
# mkdir -p /tmp/input
# while true; do xwd -root | convert - -scale 640 /tmp/input/bla.jpg; sleep 0.5; done &
## Then the files can be read from the folder "/tmp/input" and served via HTTP
# ./mjpg_streamer -i "input_file.so -f /tmp/input -r" -o "output_http.so -w www"

## To upload files to a FTP server (edit the script first)
# ./mjpg_streamer -i input_testpicture.so -o "output_file.so --command plugins/output_file/examples/ftp_upload.sh"

## To create a control only interface useful for controlling the pan/tilt throug
## a webpage while another program streams video/audio, like skype.
#./mjpg_streamer -i "./input_control.so" -o "./output_http.so -w ./www"


