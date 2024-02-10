int fileio_dir_exists(const char *mydir);
int fileio_file_exists(char *myfile);
int fileio_dirname(char *dname, char *fpath);
int fileio_basename(char *dname, char *fpath);
int fileio_rmdir(const char *mydir);
int fileio_rm(const char *myfile);
int fileio_mkdir_p(const char *mydir);
int fileio_cp(char *src, char *dest);
int fileio_mv(const char *src, const char *dest);

