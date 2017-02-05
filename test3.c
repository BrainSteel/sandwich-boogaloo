#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
printf("Begin\n");    

printf("Mid\n");    
PlaySound("C:\\Windows\\Media\\Windows Logoff Sound.wav", NULL, SND_ASYNC);
PlaySound("C:\\Windows\\Media\\Ring05.wav", NULL, SND_LOOP | SND_ASYNC);
printf("End\n");
getchar(); 
return 0;
}