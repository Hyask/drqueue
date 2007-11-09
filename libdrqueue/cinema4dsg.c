//
// Copyright (C) 2007 Andreas Schroeder
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
// USA
//
// $Id: cinema4dsg.c 2088 2007-08-22 22:07:25Z kaazoo $
//

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "cinema4dsg.h"
#include "libdrqueue.h"

char *cinema4dsg_create (struct cinema4dsgi *info) {
  /* This function creates the cinema4d render script based on the information given */
  /* Returns a pointer to a string containing the path of the just created file */
  /* Returns NULL on failure and sets drerrno */
  FILE *f;
  FILE *etc_cinema4d_sg;   /* The cinema4d script generator configuration file */
  int fd_etc_cinema4d_sg,fd_f;
  static char filename[BUFFERLEN];
  char fn_etc_cinema4d_sg[BUFFERLEN]; /* File name pointing to DRQUEUE_ETC/cinema4d.sg */
  char buf[BUFFERLEN];
  int size;
  char *p;   /* Scene filename without path */
  char scene[MAXCMDLEN];

  /* Check the parameters */
  if (!strlen(info->scene)) {
    drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

#ifdef __CYGWIN
  cygwin_conv_to_posix_path(info->scene, scene);
#else

  strncpy(scene,info->scene,MAXCMDLEN-1);
#endif

  p = strrchr(scene,'/');
  p = ( p ) ? p+1 : scene;
  snprintf(filename,BUFFERLEN-1,"%s/%s.%lX",info->scriptdir,p,(unsigned long int)time(NULL));

  if ((f = fopen (filename, "a")) == NULL) {
    if (errno == ENOENT) {
      /* If its because the directory does not exist we try creating it first */
      if (mkdir (info->scriptdir,0775) == -1) {
        drerrno = DRE_COULDNOTCREATE;
        return NULL;
      } else if ((f = fopen (filename, "a")) == NULL) {
        drerrno = DRE_COULDNOTCREATE;
        return NULL;
      }
    } else {
      drerrno = DRE_COULDNOTCREATE;
      return NULL;
    }
  }

  fchmod (fileno(f),0777);

  /* So now we have the file open and so we must write to it */
  fprintf(f,"#!/bin/tcsh\n\n");
  fprintf(f,"set SCENE=\"%s\"\n",info->scene);
 
  // 2 means we want to distribute one single image
  if (info->kind == 2) {
  	snprintf(fn_etc_cinema4d_sg,BUFFERLEN-1,"%s/cinema4d_image.sg",getenv("DRQUEUE_ETC"));
  // 1 means we want to render an animation
  } else if (info->kind == 1) {
  	snprintf(fn_etc_cinema4d_sg,BUFFERLEN-1,"%s/cinema4d.sg",getenv("DRQUEUE_ETC"));
  } else {
  	drerrno = DRE_NOTCOMPLETE;
    return NULL;
  }

  fflush (f);

  if ((etc_cinema4d_sg = fopen (fn_etc_cinema4d_sg,"r")) == NULL) {
    fprintf(f,"\necho -------------------------------------------------\n");
    fprintf(f,"echo ATTENTION ! There was a problem opening: %s\n",fn_etc_cinema4d_sg);
    fprintf(f,"echo So the default configuration will be used\n");
    fprintf(f,"echo -------------------------------------------------\n");
    fprintf(f,"\n\n");
    fprintf(f,"cinema4d -nogui -render $SCENE -frame $FRAME\n\n");
  } else {
    fd_etc_cinema4d_sg = fileno (etc_cinema4d_sg);
    fd_f = fileno (f);
    while ((size = read (fd_etc_cinema4d_sg,buf,BUFFERLEN)) != 0) {
      write (fd_f,buf,size);
    }
    fclose(etc_cinema4d_sg);
  }

  fclose(f);

  return filename;
}


char *cinema4dsg_default_script_path (void) {
  static char buf[BUFFERLEN];
  char *p;

  if (!(p = getenv("DRQUEUE_TMP"))) {
    return ("/drqueue_tmp/not/set/report/bug/please/");
  }

  if (p[strlen(p)-1] == '/')
    snprintf (buf,BUFFERLEN-1,"%s",p);
  else
    snprintf (buf,BUFFERLEN-1,"%s/",p);

  return buf;
}