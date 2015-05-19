/* 
    $Id: http download source code $ 
     get from internet
     happysu@petkit  2015-05-19
*/  
#include "http_download.h"
  
 

#define HERROR(exp, err)    do {                                                         \
                                if (exp) {                                               \
                                    printf("(%s,%d) %s\n", __FILE__, __LINE__, #exp);    \
                                    if(S_http_callback) S_http_callback((err), 0, 0, 0); \
                                    exit(1);                                             \
                                }                                                        \
                            } while(0)                            
  
#define HCALLBACK(a, b, c, d)       (S_http_callback ? S_http_callback((a), (b), (c), (d)) : 0)  
  
static HTTPDOWNLOAD_CALLBACK    S_http_callback = NULL;  
static const char * S_request_head = "GET %s HTTP/1.1\r\n" \
                                     "Accept: */*\r\n" \
                                     "Accept-Language: zh-cn\r\n" \
                                     "User-Agent: Molliza/4.0(compatible;MSIE6.0;windows NT 5.0)\r\n" \
                                     "Host: %s\r\n" \
                                     "Connection: close\r\n\r\n";
  
#if defined(__GNUC__)  
unsigned int get_tick()   
{  
    struct timeval tv;  
    gettimeofday(&tv, NULL);  
    return (unsigned int)(tv.tv_sec*1000+tv.tv_usec/1000);  
}  
#elif defined(_WIN32)  
#define get_tick()      GetTickCount()  
#endif  
  
  
static char * r_strchr(const char *p, int len, char ch)  
{  
    int last_pos = -1;  
    int i = 0;  
    while ( *(p+i) != '\0' ) {  
        if ( *(p+i) == ch ) last_pos = i;  
        if ( ++i >= len ) break;   
    }  
    return last_pos >= 0 ? (char *)(p+last_pos) : NULL;  
}  
  

static void parse_url(const char *url, char *web, int *port, char *filepath, char *filename)  
{  
    int web_len = 0;  
    int web_start = 0;  
    int file_len = 0;  
    int file_start = 0;  
    int path_len = 0;  
    int path_start = 0;  
    char *pa = NULL;  
    char *pb = NULL;  
    if ( !strncmp(url, "http://", 7) ) web_start = 7;  
    else if ( !strncmp(url, "https://", 8) ) web_start = 8;  
      
    pa = strchr(url+web_start, '/');  
    pb = strchr(url+web_start, ':');  
      
    if ( pa ) {  
        web_len = (int)(pa - (url+web_start));  
    } else {  
        if ( pb ) 
			web_len = pb - (url+web_start);  
        else 
			web_len = strlen(url+web_start);         
    }  
    HERROR(0 == web_len, HDL_URLERR);  
    memcpy(web, url+web_start, web_len);  
    web[web_len] = '\0';          

    if ( pb ) {  
        pa = r_strchr(url+web_start, (int)(pb-(url+web_start)), '/');  
        file_len = (int)(pb - pa - 1);  
        file_start = (int)(pa + 1 - url);  
        *port = atoi(pb+1);  
    } else {  
       pa = r_strchr(url+web_start, strlen(url+web_start), '/');  
        HERROR(NULL == pa, HDL_URLERR);  
        file_len = strlen(pa+1);  
        file_start = (int)(pa + 1 - url);  
        *port = DEF_HTTP_PORT;  
    }  
    HERROR(NULL == pa, HDL_URLERR);  
    memcpy(filename, url+file_start, file_len);  
    filename[file_len] = '\0';  
      
    path_start = web_start + web_len;  
    path_len = file_start - path_start + file_len;  
    memcpy(filepath, url+web_start+web_len, file_start - path_start + file_len);  
    filepath[path_len] = '\0';  
}  
  

int package_request(char *request, const char *web, const char *filepath)  
{  
    int n = sprintf(request, S_request_head, filepath, web);  
    request[n] = '\0';  
    return n;  
}  

int send_request(HSOCKET sock, const char *buf, int n)  
{  
    int already_bytes = 0;  
    int per_bytes = 0;    
      
    while ( already_bytes < n ) {  
        per_bytes = send(sock, buf+already_bytes, n-already_bytes, 0);  
        if ( per_bytes < 0 ) {  
            break;  
        } else if ( 0 == per_bytes ) {  
            HCALLBACK(HDL_DISCONN, 0, 0, 0);  
            break;  
        } else  
            already_bytes += per_bytes;   
    }  
    return already_bytes;  
}  
  

int get_response_head(HSOCKET sock, char *response, int max)  
{  
    int total_bytes = 0;  
    int per_bytes = 0;  
    int over_flag = 0;  
    char ch;  
      
    do {          
        per_bytes = recv(sock, &ch, 1, 0);  
        if ( per_bytes < 0 ) break;  
        else if ( 0 == per_bytes ) {  
            HCALLBACK(HDL_DISCONN, 0, 0, 0);  
            break;  
        } else {  
            if ( '\r' == ch || '\n' == ch ) over_flag++;  
            else over_flag = 0;  
              
            response[total_bytes] = ch;           
            if ( total_bytes >= max ) break;  
            else total_bytes += per_bytes;  
        }  
    } while ( over_flag < 4 );  
    response[total_bytes] = '\0';  
    return total_bytes;  
}  
void parse_response_head(char *response, int n, int *server_status, unsigned int *down_total)  
{  
    int i = 1;  
    char *pstatus = strstr(response, "HTTP/1.1");  
    char *plen = strstr(response, "Content-Length:");     
    printf("%s\n", response);  
      
    if ( pstatus ) {  
        pstatus += strlen("HTTP/1.1");  
        i = 1;    
        while ( *(pstatus+i) == ' ' ) ++i;  
        *server_status = atoi(pstatus+i);  
    } else  
        *server_status = 0;  
    if ( plen ) {  
        plen += strlen("Content-Length:");  
        i = 1;  
        while ( *(plen+i) == ' ' ) ++i;  
        *down_total = (unsigned int)atoi(plen+i);   
    } else  
        *down_total = 0;  
    printf("status: %d\n", *server_status);  
    printf("total: %u\n", *down_total);  
}  

FILE * create_local_file(const char *local_path, const char *local_file, const char *remote_file)  
{
    int len = strlen(local_path);  
    static char filename[HTTP_MAX_PATH+1];  
    if ( local_path[len-1] != PATH_SPLIT ) {  
        if ( local_file ) len = sprintf(filename, "%s%c%s", local_path, PATH_SPLIT, local_file);      
        else len = sprintf(filename, "%s%c%s", local_path, PATH_SPLIT, remote_file);          
    } else {  
        if ( local_file ) len = sprintf(filename, "%s%s", local_path, local_file);    
        else len = sprintf(filename, "%s%s", local_path, remote_file);    
    }  
    filename[len] = '\0';   
    printf("ready to create local file: %s\n", filename);  
    return fopen(filename, "wb+");  
}  

void start_download(HSOCKET sock, unsigned int total_bytes, FILE *stream)  
{  
    unsigned int already_bytes;  
    int per_bytes;  
    int flush_bytes;  
    unsigned int total_tim;  
    unsigned int per_tim;  
    static char buf[HTTP_DOWN_PERSIZE];  
      
    do {  
        per_tim = get_tick();  
        per_bytes = recv(sock, buf, HTTP_DOWN_PERSIZE, 0);  
        per_tim = get_tick() - per_tim;  
        if ( per_bytes < 0 ) break;  
        else if ( 0 == per_bytes ) {  
            HCALLBACK(HDL_DISCONN, total_bytes, already_bytes, 0);  
            break;        
        } else {  
            fwrite(buf, 1, per_bytes, stream);  
            already_bytes += (unsigned int)per_bytes;  
              
            flush_bytes += per_bytes;  
            if ( flush_bytes >= HTTP_FLUSH_BLOCK ) {  
                fflush(stream);  
                flush_bytes = 0;      
            }  
            HCALLBACK(HDL_DOWNING, total_bytes, already_bytes, per_tim);  
        }  
    } while ( already_bytes < total_bytes );  
      
    if ( flush_bytes > 0 )  
        fflush(stream);  
  
    HCALLBACK(HDL_FINISH, total_bytes, already_bytes, total_tim);  
}  
  
void http_download(const char *url, const char *local_path, const char *local_file, HTTPDOWNLOAD_CALLBACK http_func)  
{  
    int port;  
    char web[HTTP_MAX_PATH+1];  
    char remote_path[HTTP_MAX_PATH+1];  
    char remote_file[HTTP_MAX_PATH+1];  
    char request[HTTP_MAX_REQUESTHEAD+1];  
    char response_head[HTTP_MAX_RESPONSEHEAD+1];  
    int request_len = 0;  
    int response_len = 0;  
    unsigned int down_total = 0;  
    int server_status = 404;  
    HSOCKET sock = -1;  
    struct hostent *host = NULL;  
    struct sockaddr_in server_addr;   
    FILE *stream = NULL;  
    S_http_callback = http_func;  
    parse_url(url, web, &port, remote_path, remote_file);  
    request_len = package_request(request, web, remote_path);
    host = gethostbyname(web);  
    HERROR(NULL==host, HDL_SERVERFAL);  
      
    sock = socket(AF_INET, SOCK_STREAM, 0);  
    HERROR(-1==sock, HDL_SOCKFAL);  
      
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;  
    server_addr.sin_port = htons(port);  
    server_addr.sin_addr = *((struct in_addr *)host->h_addr_list[0]);  
    HERROR(-1==connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)), HDL_TIMEOUT);  
    HERROR(request_len != send_request(sock, request, request_len), HDL_SNDREQERR);  
    response_len = get_response_head(sock, response_head, HTTP_MAX_RESPONSEHEAD);  
    HERROR(response_len <= 0, HDL_NORESPONSE);  
    parse_response_head(response_head, response_len, &server_status, &down_total);  
    switch ( server_status ) {  
        case 200:  
            if ( down_total > 0 ) {  
                stream = create_local_file(local_path, local_file, remote_file);  
                HERROR(NULL == stream, HDL_CRLOCFILEFAL);  
                HCALLBACK(HDL_READYOK, down_total, 0, 0);  
                start_download(sock, down_total, stream);  
                fclose(stream);       
            } else {  
                HCALLBACK(HDL_FINISH, 0, 0, 0);  
            }  
        default:  
            HCALLBACK(HDL_SERVERERR, 0, 0, 0);  
    }  
    closesocket(sock);  
}  
  
 
/*********************************************************
*
*followed example code.
*
*
*********************************************************/

#if 0
#include <stdio.h>  
#include "httpdownload.h"  
int http_callback(int status, unsigned int total_bytes, unsigned int already_bytes, unsigned int tim)  
{  
    switch ( status ) {  
        case HDL_READYOK:  
            printf("ready to download, total.bytes=%u/n", total_bytes);  
            break;  
        case HDL_DOWNING:  
            printf("downloading... (%u/%u) bytes/n", already_bytes, total_bytes);  
            break;  
        case HDL_FINISH:  
            printf("download finish! download total.bytes=%u/n", already_bytes);  
            break;  
        default:  
            printf("status: %#x/n", status);      
    }     
    return 0;  
}  
  
int main(int argc, char *argv[])  
{  
    http_download(argv[1], argv[2], NULL, http_callback);  
}  

#endif
