int bmkdir_p(const char *path_to_dir,int mode);
char *sfindductype(char filepath[]);
int rrmdir(char dir[]);

int readfile_into_buf( char *filename, char **buf );

FILE *stretchfile(FILE *FH,char mode[],char file[], off_t branksize);

int closeAtExexo(int fd);
int fcloseAtExexo(FILE *FILEHANDLER);
