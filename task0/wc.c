#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>

int total_ccount=0;
int total_wcount=0;
int total_lcount=0;

int single_ccount,single_wcount,single_lcount;

char * result_fname="result.txt";

void print_help()
{
	fprintf(stderr,"usage wc [file ...]\n");
}

void wc(FILE *ofile, FILE *infile, char *inname) {
	char c;
	char ctemp=0;
	//ctemp as a temp value to restore the first character in a word
	while((c=fgetc(infile)) != EOF)
	{
		single_ccount++;
		if(c == '\n')
			single_lcount++;
		if(isalpha(c) && !isalpha(ctemp))
			ctemp=c;
		if(isalpha(ctemp) && !isalpha(c))
		{
			single_wcount++;
			ctemp=0;
		}
	}
	total_ccount+=single_ccount;
	total_wcount+=single_wcount;
	total_lcount+=single_lcount;
	printf("#%s# characters:%d words:%d lines:%d\n",
			inname,single_ccount,single_wcount,single_lcount);
	fseek(ofile,0L,SEEK_END);
#if 0
	long pos;
	pos=ftell(ofile);
	printf("pos:%d\n",pos);
#endif
	fprintf(ofile,"#%s# characters:%d words:%d lines:%d\n",
			inname,single_ccount,single_wcount,single_lcount);
}

int main (int argc, char *argv[]) {
	if(argc<2)
	{
		print_help();
		return -2;
	}

	#define BUFFSIZE 1024
	char error_buf[BUFFSIZE];

	FILE *outfp=fopen(result_fname,"w+");
	if(!outfp)
	{
		sprintf(error_buf,"fail to operate file#%s#",result_fname);
		perror(error_buf);
		exit(-3);
	}

	single_ccount=single_wcount=single_lcount=0;

	int i;
	for(i=1;i<argc;i++)
	{
		char * fname=argv[i];
		FILE *infp=fopen(fname,"r");
		if(!infp)
		{
			sprintf(error_buf,"fail to open file #%s#",fname);
			perror(error_buf);
			exit(-4);
		}
		wc(outfp,infp,fname);
		fclose(infp);
	}

	if(argc>2)
	{
		fseek(outfp,0L,SEEK_END);
		printf("#Total# characters:%d words:%d lines:%d\n",
				total_ccount,total_wcount,total_lcount);
		fprintf(outfp,"#Total# characters:%d words:%d lines:%d\n",
				total_ccount,total_wcount,total_lcount);
	}

	fclose(outfp);
    return 0;
}
