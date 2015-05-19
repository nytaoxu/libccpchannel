#ifndef _HTTP_DOWNLOAD_H_
#define _HTTP_DOWNLOAD_H_
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <errno.h> 
#if defined(__GNUC__)  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <unistd.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>  
#include <netdb.h>
#include <time.h> 
#include <sys/time.h>

#define PATH_SPLIT          '/'  
typedef int HSOCKET;  
#define closesocket(s)      close(s)  
#elif defined(_WIN32)  
#include <windows.h>  
#include <winsock2.h>  
#define PATH_SPLIT          '//'      
typedef SOCKET HSOCKET;  
#endif  
  
#define DEF_HTTP_PORT           80  
#define HTTP_MAX_PATH           260  
#define HTTP_MAX_REQUESTHEAD    1024  
#define HTTP_MAX_RESPONSEHEAD   2048  
#define HTTP_DOWN_PERSIZE       128   
#define HTTP_FLUSH_BLOCK        1024  
  
/* 
    download status
    HDL_URLERR: url invalid 
    $@ HDL_SERVERFAL: can not get ip from dns  
    $@ HDL_TIMEOUT: connect server timeout 
    $@ HDL_READYOK: connect success,ready to download
    $@ HDL_DOWNING: downloading 
    $@ HDL_DISCONN: disconnected 
    $@ HDL_FINISH: download complete 
*/  
enum   
{  
    HDL_URLERR = 0xe0,  
    HDL_SERVERFAL = 0xe1,  
    HDL_SOCKFAL = 0xe2,  
    HDL_SNDREQERR = 0xe3,  
    HDL_SERVERERR = 0xe4,  
    HDL_CRLOCFILEFAL = 0xe5,  
    HDL_TIMEOUT = 0x100,  
    HDL_NORESPONSE = 0x101,  
    HDL_READYOK = 0x104,  
    HDL_DOWNING = 0x105,  
    HDL_DISCONN = 0x106,  
    HDL_FINISH = 0x107  
};  
/* 
    download callback, used to report download procedure
    $@1,download status enum value 
    $@2,total download file bytes. 
    $@3,complete download file bytes 
    $@4,timestamp 
    $@5,return value: 
*/
typedef int (*HTTPDOWNLOAD_CALLBACK)(int, unsigned int, unsigned int, unsigned int);  
 
 /* 
	 $@@download file via http protocol. 
	 $@ url: download url
	 $@ filepath: local file path,if it is null,save to current directory.
	 $@ filename: local file name,if it is null,use file name in url. 
	 $@ http_func: callback,can be null 
 */  
 void http_download(const char *url, const char *filepath, const char *filename, HTTPDOWNLOAD_CALLBACK http_func);	

#endif
