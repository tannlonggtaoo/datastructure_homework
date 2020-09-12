#pragma once
//请务必把电器设备表文件转换为国标（GBK（GB2013））编码（windows10），转为与控制台相对应的编码
#define MAX_PATH_LENGTH 70
//缓存区大小
#define BUF_SIZE 50
//节点/等电位点大小
#define BUSBUF_SIZE 50
#define BUSBUF_INC 10
//函数工作返回值
enum Status {OK,ERROR};