#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}

#define LS_IS_MORE
#ifdef LS_IS_MORE
char
filetype(int sttype)
{
    char ftype = '-';

    switch (sttype) {
    case T_DIR:
        ftype = 'd';
        break;
    case T_FILE:
        ftype = 'f';
        break;
    case T_DEV:
        ftype = 'D';
        break;
    default:
        ftype = '?';
        break;
    }
    return ftype;
}
#endif // LS_IS_MORE

void
ls(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
#ifdef LS_IS_MORE
      printf(1, "%s %c %d %d %d\n", fmtname(path), filetype(st.type), st.nlink, st.ino, st.size);
#else // LS_IS_MORE
    printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
#endif // 
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf(1, "ls: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf(1, "ls: cannot stat %s\n", buf);
        continue;
      }
#ifdef LS_IS_MORE

      printf(1, "%s %c %d %d %d\n", fmtname(buf), filetype(st.type), st.nlink, st.ino, st.size);
#else // LS_IS_MORE
      printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
#endif
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    ls(".");
    exit();
  }
  for(i=1; i<argc; i++)
    ls(argv[i]);
  exit();
}
