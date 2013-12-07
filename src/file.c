#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined( __APPLE__ )
  #include <CommonCrypto/CommonDigest.h>

  #ifdef MD5_DIGEST_LENGTH
    #undef MD5_DIGEST_LENGTH
  #endif

  #define MD5_Init            CC_MD5_Init
  #define MD5_Update          CC_MD5_Update
  #define MD5_Final           CC_MD5_Final
  #define MD5_DIGEST_LENGTH   CC_MD5_DIGEST_LENGTH
  #define MD5_CTX             CC_MD5_CTX
#else
  #include <openssl/md5.h>
#endif

#include "include/file.h"
#include "include/remodel.h"

char* read_file(FILE* file) {
  struct stat stat;
  if (fstat(fileno(file), &stat) < 0) {
    fprintf(stderr, "fstat: %s\n", strerror(errno));
    return NULL;
  }

  uint64_t fsize = (uint64_t)stat.st_size;
  char* str = calloc(1, fsize + 1);
  assert(str);

  if (fread(str, fsize, 1, file) != 1 && ferror(file) != 0) {
    if (options.debug) {
      fprintf(stderr, "[remodel] fread: %s\n", strerror(errno));
    }
    return NULL;
  }

  return str;
}

char* md5sum(FILE* file) {
  uint8_t md5[MD5_DIGEST_LENGTH];
  uint8_t buf[1024];
  uint32_t num_bytes;
  MD5_CTX ctx;

  MD5_Init(&ctx);
  while ((num_bytes = fread(buf, 1, 1024, file)) != 0) {
    MD5_Update(&ctx, buf, num_bytes);
  }
  MD5_Final(md5, &ctx);

  char* res = malloc(2*MD5_DIGEST_LENGTH+1);
  for (uint32_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
    sprintf(&res[2*i], "%02x", md5[i]);
  }
  return res;
}

// awful
void mkdirp(char* path, mode_t mode) {
  char* filename = strrchr(path, '/');
  if (filename == NULL) {
    return;
  }
  filename[0] = '\0';

  char* p = path;
  while (*p != '\0') {
    do {
      p++;
    } while (*p != '\0' && *p != '/');

    char v = *p;
    *p = '\0';

    if(mkdir(path, mode) == -1 && errno != EEXIST) {
      fprintf(stderr, "error: mkdir(%s): %s\n", p, strerror(errno));
      *p = v;
      break;
    }

    *p = v;
  }

  filename[0] = '/';
}

bool file_changed(const char* path) {
  bool res;
  char* old_md5 = NULL;
  char* tmp_cache_path = NULL;

  FILE* file = fopen(path, "r+");
  if (file == NULL) {
    if (options.debug) {
      fprintf(stderr, "[remodel] fopen(%s): %s\n", path, strerror(errno));
    }
    return true;
  }
  char* current_md5 = md5sum(file);

  uint32_t cache_path_len = strlen(REMODEL_DIR) + 1 + strlen(path) + 1;
  char* cache_path = malloc(cache_path_len);
  snprintf(cache_path, cache_path_len, "%s/%s", REMODEL_DIR, path);

  mkdirp(cache_path, 0755);
  FILE* cache_file = fopen(cache_path, "r");
  if (cache_file == NULL) {
    if (options.debug) {
      fprintf(stderr, "[remodel] fopen(%s): %s\n", cache_path, strerror(errno));
    }
    res = true;
  } else {
    old_md5 = read_file(cache_file);
    if(old_md5 == NULL) {
      fprintf(stderr, "[remodel] fread(%s): %s\n", cache_path, strerror(errno));
      res = true;
      goto exit;
    }

    res = strcmp(old_md5, current_md5) != 0;

    if (options.debug) {
      fprintf(stderr, "[remodel] %s: md5=%s, cache=%s\n", path, current_md5, old_md5);
    }
  }

  // try to open the cache file for writing, and write the current md5 sum back to the cache.
  // it is possible for multiple threads to try and check if a file has changed concurrently
  // (e.g. bar.c <- foo.c, baz.c <- foo.c) to deal with this, we write the new md5 sum to a
  // tmp file with the current thread's identifier and rename it to the original file name.
  uint32_t tmp_cache_path_len = cache_path_len + 1 + 3 + 1 + 10;
  tmp_cache_path = malloc(tmp_cache_path_len + 1);
  sprintf(tmp_cache_path, "%s.tmp.%p", cache_path, pthread_self());
  FILE* tmp_cache_file = fopen(tmp_cache_path, "w");
  if (!tmp_cache_file) {
    goto exit;
  }

  if (fwrite(current_md5, strlen(current_md5), 1, tmp_cache_file) == 0) {
    fprintf(stderr, "error: fwrite: %s\n", strerror(errno));
  }

  if (rename(tmp_cache_path, cache_path) != 0) {
    fprintf(stderr, "error: rename(%s, %s): %s\n",
            tmp_cache_path, cache_path, strerror(errno));
  }

  fclose(tmp_cache_file);

exit:
  fclose(cache_file);
  fclose(file);
  free(tmp_cache_path);
  free(cache_path);
  free(current_md5);
  free(old_md5);

  return res;
}
