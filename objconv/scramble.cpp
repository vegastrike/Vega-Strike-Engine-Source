#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
int main (int argc, char ** argv) {
	if (argc<2) {
		printf ("require arg");
		return 0;
	}
	printf ("are you sure you wish to nail %s\n",argv[1]);
	char c;
	scanf ("%c",&c);
	if (c!='y'&&c!='Y')
		return 0 ;
	int fp = open (argv[1],O_RDWR);
	struct stat s;
	fstat (fp,&s);
	size_t size=s.st_size;
	char * mem = (char *)mmap (NULL,size,PROT_READ|PROT_WRITE,MAP_SHARED,fp,0);
	int sum=0;
	FILE * rand = fopen ("/dev/random","r");
	for (size_t i=0;i<size;++i) {
		sum+=mem[i];
		mem[i]=getc(rand);
	}
	fclose(rand);
	munmap(mem,size);
	close(fp);
	return 0;
}
