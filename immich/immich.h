#include <sys/stat.h>
#include <stdio.h>

struct immichFile {
	struct stat st;
	FILE *file;
	char *fpath;
	char checksum[40];
	char assetId[1024];
};

struct immichConn {
    char auth[43];
    char *url;
};

int immich_upload(struct immichConn*, struct immichFile*);
