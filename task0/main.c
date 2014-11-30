#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>


int main() {
    struct rlimit lim;
	getrlimit(RLIMIT_STACK,&lim);
    printf("stack size: %d\n",  (int)lim.rlim_cur);
	getrlimit(RLIMIT_NPROC,&lim);
    printf("process limit: %lld\n", (long long int)lim.rlim_cur);
	getrlimit(RLIMIT_FSIZE,&lim);
    printf("max file descriptors: %lld\n", (long long int)lim.rlim_cur);

	//doubt:sizeof(rlim_t)==sizeof(int),but if i use int to printf
	//max fd,it is -1,why? is -1 means unlimited?
	printf("sizeof(long long int):%d sizeof(int):%d sizeof(rlim_t):%d\n",
			sizeof(long long int), sizeof(int), sizeof(rlim_t));

	getrlimit(RLIMIT_CPU,&lim);
	printf("default time limit is: %lld the hard limit is: %lld\n",
			(long long int)lim.rlim_cur, (long long int)lim.rlim_max);

	lim.rlim_cur=10; //10s
	setrlimit(RLIMIT_CPU,&lim);

	getrlimit(RLIMIT_CPU,&lim);
	printf("time limit now is: %lld",(long long int)lim.rlim_cur);
	
	while(1);

	return 0;
}
