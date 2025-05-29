#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <3ds.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include <3ds.h>
// #include <3ds/sdmc.h>

#include <curl/curl.h>

#include <dirent.h>

#include "immich/immich.h"

#include "config.h"

PrintConsole g_statusConsole;
PrintConsole g_logConsole;
PrintConsole g_sessionConsole;


int cnt;

int countLines(char* str)
{
	if(!str)return 0;
	int cnt; for(cnt=1;*str=='\n'?++cnt:*str;str++);
	return cnt;
}

void cutLine(char* str)
{
	if(!str || !*str)return;
	char* str2=str;	for(;*str2&&*(str2+1)&&*str2!='\n';str2++);	str2++;
	memmove(str,str2,strlen(str2)+1);
}

// void drawFrame()
// {
// 	gfxFlushBuffers();
// 	gfxSwapBuffers();
// }

int colour = 1;

#define SOC_BUFFERSIZE  0x100000
#define SOC_ALIGN       0x1000

#include <stdlib.h>

static u32 *SOC_buffer = NULL;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <netdb.h>

int mystat(char *fname, struct stat *st) {
	long int ret;
	ret = stat(fname, st);
	if (ret != 0)
		printf("stat failed L%d: %" PRId32 "\n", __LINE__, ret);
	u64 timestamp;
	ret = archive_getmtime(fname, &timestamp);
	if (ret != 0)
		printf("return from archive_getmtime L%d: %" PRId32 "\n", __LINE__, ret);
	st->st_mtim.tv_sec = timestamp; // this is gonna fail in 2038 lol
	return 0;
}

int dothedirs(char *dpath) {
	DIR *d = opendir(dpath);
	int count = 0;

	struct immichConn conn = {
		.url=IMMICH_URL,
		.auth=IMMICH_KEY
	};

	if (d) {
		struct dirent *dir;
		while ((dir = readdir(d)) != NULL) {
			struct immichFile f;
			char fullname[2024];
			count++;
			if (count < 25) continue;
			sprintf(fullname, "%s/%s", dpath, dir->d_name);
			f.fpath = fullname;
			mystat(fullname, &f.st);
			sprintf(f.assetId, "%s-%lld", dir->d_name, f.st.st_size);
			f.file = fopen(f.fpath, "r");


			fprintf(stderr, "%.8s: ", f.assetId);
			int ret = immich_upload(&conn, &f);
			fclose(f.file);
			fprintf(stderr, "\n");
			if (ret != 0) {
				break;
			}
			// max size: 90000

			// printf("%s\n", dir->d_name);
			// if (count > 40) break;
		}
		closedir(d);
	}
}

int main()
{
	// Initialize services
	srvInit();
	aptInit();
	hidInit();
	gfxInitDefault();
	//gfxSet3D(true); // uncomment if using stereoscopic 3D
	
	// consoleInit (GFX_TOP, &g_statusConsole);
	// consoleInit (GFX_TOP, &g_logConsole);
	consoleInit (GFX_BOTTOM, &g_sessionConsole);

	// width, height
	// consoleSetWindow (&g_statusConsole, 0, 0, 32, 1);
	// consoleSetWindow (&g_logConsole, 0, 1, 32, 23);
	// console is 40x25 maybe
	// consoleSetWindow (&g_sessionConsole, 0, 0, 150, 25);
	consoleSetWindow (&g_sessionConsole, 0, 0, 40, 25);

	// consoleDebugInit (debugDevice_SVC);
	consoleDebugInit(debugDevice_CONSOLE);
	setvbuf (stderr, NULL, _IONBF, 0);


	long int ret;
	// ret = httpcInit(0x3000);
	// if (ret != 0)
	// 	printf("return from httpcInit L%d: %" PRId32 "\n", __LINE__, ret);

	// ret=http_download("http://test30.requestcatcher.com/3ds-test-polo");
	// if (ret != 0)
	// 	printf("return from http_download L%d: %" PRId32 "\n", __LINE__, ret);

	ret = fsInit();
	if (ret != 0)
		printf("return from fsInit L%d: %" PRId32 "\n", __LINE__, ret);

	// ret = archiveMountSdmc();
	// if (ret != 0)
	// 	printf("return from archiveMountSdmc L%d: %" PRId32 "\n", __LINE__, ret);

	SOC_buffer = (u32*)aligned_alloc(SOC_ALIGN, SOC_BUFFERSIZE);

	if(SOC_buffer == NULL) {
		printf("wuhoh! memalign failed\n");
		// failExit("memalign: failed to allocate\n");
	}

	if ((ret = socInit(SOC_buffer, SOC_BUFFERSIZE)) != 0) {
		printf("wuhoh!\n");
		// failExit("socInit: 0x%08X\n", (unsigned int)ret);
    }

	// ret = sdmcInit();
	// if (ret != 0)
	// 	printf("return from sdmcInit L%d: %" PRId32 "\n", __LINE__, ret);

	/*
	FS_Archive sdmcArchive;
	ret = FSUSER_OpenArchive(&sdmcArchive, ARCHIVE_SDMC, fsMakePath(PATH_EMPTY, ""));
	if (ret != 0)
		printf("return from FSUSER_OpenArchive L%d: %" PRId32 "\n", __LINE__, ret);

	// sdmc:/
	Handle dir;
	ret = FSUSER_OpenDirectory(&dir, sdmcArchive, fsMakePath(PATH_ASCII, "/DCIM/105NIN03/"));
	if (ret != 0)
		printf("return from FSUSER_OpenDirectory L%d: %" PRId32 "\n", __LINE__, ret);

	u32 entry_count = 0;
	u32 count = 0;
	do {
		FS_DirectoryEntry entry;

		if (R_FAILED(ret = FSDIR_Read(dir, &entry_count, 1, &entry))) {
			printf("return from FSDIR_Read L%d: %" PRId32 "\n", __LINE__, ret);
			break;
		}

		count++;
		doFile(sdmcArchive, entry);
		if (count > 5) break;
		// printf("file found: %s\n", entry.shortName);
	} while(entry_count > 0);
	*/

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		// Your code goes here


		u8* bufAdr=gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		int i, j;

		for(i=1;i<400;i++)
		{
			for(j=1;j<240;j++)
			{
				u32 v=(j+i*240)*3;
				if (colour == 1) //blue
				{
					//lettuce set screen to blue
					bufAdr[v] = 0xFF;
					bufAdr[v+1] = 0x00;
					bufAdr[v+2] = 0x00;
				}

				if (colour == 2) //green
				{
					bufAdr[v] = 0x00;
					bufAdr[v+1] = 0xFF;
					bufAdr[v+2] = 0x00;
				}

				if (colour == 3) //red
				{
					bufAdr[v] = 0x00;
					bufAdr[v+1] = 0x00;
					bufAdr[v+2] = 0xFF;
				}
			}
		}

		int k;
		int v;
		int qq;
		int ss;
		int cn;
		int sn;
		int latest;

		if (cn < 15) {
    	//Variable is not too big/NULL.
    } else {
      cn = 2;
    }

		if (sn < 15) {
			//Variable is not too big/NULL.
		} else {
			sn = 2;
		}

		if (latest < 900) {
			//Variable is not too big/NULL.
		} else {
			latest = 1;
		}


		u32 kDown = hidKeysDown();

		//Switch colors w/ R
		if (kDown & KEY_R)
		{
			if (colour < 3)
			{
				colour++;
			} else {
				colour = 1;
			}
		}



		if (kDown & KEY_L) {
			fprintf(stderr, "hello\n");
			printf("polo\n");
			dothedirs("sdmc:/DCIM/106NIN03/");
			// immich_upload(&conn, &f);
			printf("world\n");
		}



		if (kDown & KEY_START)
			break; // break in order to return to hbmenu

		// Example rendering code that displays a white pixel
		// Please note that the 3DS screens are sideways (thus 240x400 and 240x320)

		//u8* fb = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
		//memset(fb, 0, 240*400*3);
		//fb[3*(10+10*240)] = 0xFF;
		//fb[3*(10+10*240)+1] = 0xFF;
		//fb[3*(10+10*240)+2] = 0xFF;


		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();
	}

	// Exit services
exit:
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	return 0;
}
