#include <windows.h>
#include <stdio.h>

int main()
{
PlaySound("rsc\Ring05.wav", NULL, SND_LOOP | SND_ASYNC);
getchar(); 
return 0;
}