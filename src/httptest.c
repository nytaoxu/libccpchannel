#include <stdio.h>  
#include "http_download.h"  
  
int main(int argc, char *argv[])  
{  
    http_download(argv[1], argv[2], NULL, NULL);  
}  
