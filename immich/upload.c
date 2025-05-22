#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "immich.h"

#include <stdlib.h>

#include <curl/curl.h>

#include <errno.h>

static CURL *curl = NULL;

int immich_upload(struct immichConn *conn, struct immichFile *ifile) {
	CURLcode res;

	if ((res = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK) {
		fprintf(stderr, "curl_global_init() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
	}

	if (curl == NULL) {
		curl = curl_easy_init();
		if (curl == NULL) {
			return -1;
		}
	}

    int ret = 0;

	curl_mime *form = curl_mime_init(curl);
	if (form == NULL) {
		fprintf(stderr, "curl_mime_init() L%i failed\n", __LINE__);
		ret = -1;
		goto curl_cleanup;
	}

	curl_mimepart *field = NULL;

	if ((field = curl_mime_addpart(form)) == NULL) {
		fprintf(stderr, "curl_mime_addpart() L%i failed\n", __LINE__);
		ret = -1;
		goto mime_cleanup;
	};
	if ((res = curl_mime_name(field, "deviceAssetId")) != CURLE_OK) {
		fprintf(stderr, "curl_mime_name() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}
	if ((res = curl_mime_data(field, ifile->assetId, CURL_ZERO_TERMINATED)) != CURLE_OK) {; // TODO: unique based off of file
		fprintf(stderr, "curl_mime_name() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}

	if ((field = curl_mime_addpart(form)) == NULL) {
		fprintf(stderr, "curl_mime_addpart() L%i failed\n", __LINE__);
		ret = -1;
		goto mime_cleanup;
	};
	if ((res = curl_mime_name(field, "deviceId")) != CURLE_OK) {
		fprintf(stderr, "curl_mime_name() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}
	if ((res = curl_mime_data(field, "libimmich-c", CURL_ZERO_TERMINATED)) != CURLE_OK) {; // TODO: appendable based on config, user-agent
		fprintf(stderr, "curl_mime_data() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}

	if ((field = curl_mime_addpart(form)) == NULL) {
		fprintf(stderr, "curl_mime_addpart() L%i failed\n", __LINE__);
		ret = -1;
		goto mime_cleanup;
	};
	if ((res = curl_mime_name(field, "fileCreatedAt")) != CURLE_OK) {
		fprintf(stderr, "curl_mime_name() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}
	struct tm *tm_info = localtime(&ifile->st.st_mtim.tv_sec); // TODO: timezones
	char iso_mtime[28];
	strftime(iso_mtime, sizeof(iso_mtime), "%Y-%m-%dT%H:%M:%S%z", tm_info);
	if ((res = curl_mime_data(field, iso_mtime, CURL_ZERO_TERMINATED)) != CURLE_OK) {;
		fprintf(stderr, "curl_mime_data() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}

	if ((field = curl_mime_addpart(form)) == NULL) {
		fprintf(stderr, "curl_mime_addpart() L%i failed\n", __LINE__);
		ret = -1;
		goto mime_cleanup;
	};
	if ((res = curl_mime_name(field, "fileModifiedAt")) != CURLE_OK) {
		fprintf(stderr, "curl_mime_name() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}
	if ((res = curl_mime_data(field, iso_mtime, CURL_ZERO_TERMINATED)) != CURLE_OK) {;
		fprintf(stderr, "curl_mime_data() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}

	/* ignored fields rn:
	 * - isFavorite
	 * - duration
	 */


	// the meat
	if ((field = curl_mime_addpart(form)) == NULL) {
		fprintf(stderr, "curl_mime_addpart() L%i failed\n", __LINE__);
		ret = -1;
		goto mime_cleanup;
	};
	if ((res = curl_mime_name(field, "assetData")) != CURLE_OK) {
		fprintf(stderr, "curl_mime_name() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto mime_cleanup;
	}
    // ok, so do I want to use filedata or do I want to roll my own curl_mime_data_cb
	errno = 0;
	if ((res = curl_mime_filedata(field, ifile->fpath)) != CURLE_OK) {
		fprintf(stderr, "curl_mime_filedata() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		if (errno) perror("curl_mime_filedata()");
		ret = -1;
		goto mime_cleanup;
	}

	// field = curl_mime_addpart(form);
	// if ((res = curl_mime_name(field, "sidecarData");
	// curl_mime_filedata(field, "image.xmp"); // TODO: make this configurable

	struct curl_slist *headerlist = NULL;
	// To avoid overwriting an existing non-empty list on failure, the new
	// list should be returned to a temporary variable which can be tested for
	// NULL before updating the original list pointer. 
	struct curl_slist *tmpheaderlist = NULL; 

	char apiheader[43 + 13];
	sprintf(apiheader, "x-api-key: %s", conn->auth);
	
	if ((tmpheaderlist = curl_slist_append(headerlist, apiheader)) == NULL) {
		fprintf(stderr, "curl_slist_append() L%i failed\n", __LINE__);
		ret = -1;
		goto slist_cleanup;
	}
	headerlist = tmpheaderlist;
	// headerlist = curl_slist_append(headerlist, "x-immich-checksum: awawa"); // TODO

	// get /api/assets from well known maybe? TODO
	char url[1024];
	sprintf(url, "%s/api/assets", conn->url);
	if ((res = curl_easy_setopt(curl, CURLOPT_URL, url)) != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto slist_cleanup;
	}
	if ((res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist)) != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto slist_cleanup;
	}
	if ((res = curl_easy_setopt(curl, CURLOPT_MIMEPOST, form)) != CURLE_OK) {
		fprintf(stderr, "curl_easy_setopt() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		ret = -1;
		goto slist_cleanup;
	}

	// do it
	errno = 0;
	if((res = curl_easy_perform(curl)) != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() L%i failed: %s\n", __LINE__, curl_easy_strerror(res));
		if (errno) perror("curl_easy_perform()");
		ret = -1;
		goto slist_cleanup;
	}

	/* always cleanup */

    slist_cleanup:
	/* free slist */
	curl_slist_free_all(headerlist);

    mime_cleanup:
	/* then cleanup the form */
	curl_mime_free(form);

    curl_cleanup:
	// curl_easy_cleanup(curl); // TODO: I shouldn't do this, reusing sessions is good

	return ret;
}

// int main(int argc, char *argv[])
// {
// 	struct stat st;
// 	int ret;
// 
// 	struct immichFile ifile;
// 
// 	ifile.fpath = "subdir/3da9b4bc-0e5b-4433-94a3-6ca1b3701816.jpeg";
// 
// 	if (ret = stat(ifile.fpath, &ifile.st)) {
// 		perror("stat");
// 		exit(EXIT_FAILURE);
// 	};
// 	sprintf(ifile.assetId, "%s-%ld", ifile.fpath, ifile.st.st_size);
// 	ifile.file = fopen(ifile.fpath, "r");
// 
// 	struct immichConn conn = {.url="http://localhost:2283", .auth="zvRGMLWP3J9vVE7uAvOTpx0BPFfdLMSNokOs3yxMhow"};
// 
// 	immich_upload(&conn, &ifile);
// 	return 0;
// }
