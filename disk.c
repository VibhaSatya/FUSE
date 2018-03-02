#include "common.h"
#include "disk.h"

int writeLog(char *message)
{	
	const char* logFile ="logs.txt";
	time_t rawtime;
  	struct tm * timeinfo;
	char *t=(char*)malloc(50);
	time ( &rawtime );
  	timeinfo = localtime ( &rawtime );
	strcpy(t,asctime (timeinfo));
	logfile = fopen(logFile,"a+");
	if(logfile==NULL) return -1;
	fwrite("\n\n",1,1,logfile);
	t[strlen(t)-1]=' ';
	fwrite(t,1,strlen(t),logfile);
	
	fwrite(message,1,strlen(message),logfile);
	
	fclose(logfile);
	return 1;	

}



int disk_init(const char *filename, char *mode,int n )
{
        nblocks=0;
        nreads=0;
        nwrites=0;
	writeLog("Initializing disk");
	diskfile = fopen(filename,mode);
	if(!diskfile) diskfile = fopen(filename,"w+");
	if(!diskfile) {writeLog("Initialization Failed"); return 0;}

	ftruncate(fileno(diskfile),n*BLOCKSIZE);
	nblocks = n;
	nreads = 0;
	nwrites = 0;
	writeLog("Successful disk initialization");
	return 1;
}

int disk_size()
{
	return nblocks;
}

static void sanity_check( int blocknum,union fs_block *data )
{
	if(blocknum<0) {
		printf("ERROR: blocknum (%d) is negative!\n",blocknum);
		writeLog("Negative blocknum accessed");
		abort();
	}

	if(blocknum>=nblocks) {
		printf("ERROR: blocknum (%d) is too big!\n",blocknum);
		writeLog("Blocknum out of range");
		abort();
	}

	if(!data) {
		printf("ERROR: null data pointer!\n");
		writeLog("NUll data pointer");
		abort();
	}
}

void disk_read( int blocknum, union fs_block *data )
{
	//char* temp=malloc(sizeof(char)*BLOCKSIZE);
	sanity_check(blocknum,data);
	fseek(diskfile,blocknum*BLOCKSIZE,SEEK_SET);
	
	if(fread(data,BLOCKSIZE,1,diskfile)==1) {
		nreads++;writeLog("DISK READ");
	} else {
		printf("ERROR: couldn't access simulated disk: %s\n",strerror(errno));
		writeLog("Could not access disk");
		abort();
	}
}

void disk_write( int blocknum, union fs_block *data )
{
	sanity_check(blocknum,data);

	fseek(diskfile,blocknum*BLOCKSIZE,SEEK_SET);
        
	if(fwrite(data,BLOCKSIZE,1,diskfile)==1) {
		nwrites++;writeLog("DISK WRITE");
	} else {
		printf("ERROR: couldn't access simulated disk: %s\n",strerror(errno));
		writeLog("Could not access disk");
		abort();
	}
}

void disk_close()
{
	if(diskfile) {
		writeLog("Closing disk");
		printf("%d disk block reads\n",nreads);
		printf("%d disk block writes\n",nwrites);
		fclose(diskfile);
		diskfile = 0;
	}
        return;
}




