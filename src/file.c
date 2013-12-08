#include <assert.h>
#include <stdlib.h>
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
    return NULL;
  }

  return str;
}

char* md5sum(FILE* file) {
  uint8_t md5[MD5_DIGEST_LENGTH];
  uint8_t buf[1024];
  size_t num_bytes;
  MD5_CTX ctx;

  MD5_Init(&ctx);
  while ((num_bytes = fread(buf, 1, 1024, file)) != 0) {
    MD5_Update(&ctx, buf, (uint32_t)num_bytes);
  }
  MD5_Final(md5, &ctx);

  char* res = malloc(2*MD5_DIGEST_LENGTH+1);
  for (uint32_t i = 0; i < MD5_DIGEST_LENGTH; i++) {
    snprintf(&res[2*i], sizeof(&res[2*i]), "%02x", md5[i]);
  }
  return res;
}

static void mkdirp(const char* target, mode_t mode) {
  char* path = strdup(target);

  char* filename = strrchr(path, '/');
  if (filename == NULL) {
    goto exit;
  }
  filename[0] = '\0';

  char* p = path;
  while (*p != '\0') {
    do {
      p++;
    } while (*p != '\0' && *p != '/');

    char v = *p;
    *p = '\0';

    if (mkdir(path, mode) == -1 && errno != EEXIST) {
      fprintf(stderr, "error: mkdir(%s): %s\n", p, strerror(errno));
      *p = v;
      break;
    }

    *p = v;
  }

exit:
  free(path);
}

bool file_changed(const char* path) {
  bool res;

  FILE* file = fopen(path, "r");
  if (file == NULL) {
    return true;
  }
  char* current_md5 = md5sum(file);

  size_t cache_path_len = strlen(REMODEL_DIR) + 1 + strlen(path) + 1;
  char* cache_path = malloc(cache_path_len);
  snprintf(cache_path, cache_path_len, "%s/%s", REMODEL_DIR, path);

  mkdirp(cache_path, 0755);
  FILE* cache_file = fopen(cache_path, "r+");
  if (cache_file == NULL) {
    res = true;
    cache_file = fopen(cache_path, "w+");
  } else {
    char* old_md5 = read_file(cache_file);
    res = strcmp(old_md5, current_md5) != 0;
    free(old_md5);
  }

  // try to open the cache file for writing, and write the current md5 sum back to
  // the cache. it is possible for multiple threads to try and check if a file has
  // changed concurrently (e.g. bar.c <- foo.c, baz.c <- foo.c)
  fseek(cache_file, 0, SEEK_SET);
  if (fwrite(current_md5, strlen(current_md5), 1, cache_file) == 0) {
    fprintf(stderr, "error: fwrite: %s\n", strerror(errno));
  }

  fclose(cache_file);
  fclose(file);
  free(current_md5);
  free(cache_path);

  return res;
}
