#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <dirent.h>


int main(void){

	// FILE *arq;
	DIR *dir;
	struct dirent *arq;
	char buff[100];
	size_t size = 0;

	size_t count = 0;

	dir = opendir("/proc/");
	if(dir == NULL)
		puts("erro");

	while((arq = readdir(dir)) != NULL)
		printf("%s\n", arq->d_name);



	// if(readlink("/proc/2173/fd/10", buff, 100) > 0){
	// 	if(strspn(buff, "socket:"))
	// 		printf("%s", buff);
	// }



	// arq = fopen("/proc/2173/fd/10", "r");
	// if(arq == NULL)
	// 	puts("erro ao abrir arq");
	//
	//
	// while(getline(&buff, &size, arq) > 0){
	// 	count++;
	// 	if (count == 5){
	// 		count = strcspn(buff, ":");
	// 		printf("%ld\n", count);
	// 		break;
	// 	}
	// }

}
