#include <X11/Intrinsic.h>
#include <X11/Shell.h>
#include <X11/StringDefs.h>
#include <X11/Xatom.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Scrollbar.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Box.h>
#include <X11/Xaw/Toggle.h>
#include <X11/Xmu/StdSel.h>
#include <X11/Xmd.h>
#include <X11/extensions/xf86vmode.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/mman.h>


XF86VidModeGetVideo(dis, screen, addr, width, bank, ram)
Display *dis;
int screen;
char **addr;
int *width, *bank, *ram;
{
   int offset, fd;
   int pid, status;

   XF86VidModeGetVideoLL(dis, screen , &offset, width, bank, ram);


   if ((fd = open("/dev/mem", O_RDWR)) < 0)
   {
        fprintf(stderr, "XF86VidModeGetVideo: failed to open /dev/mem (%s)\n",
                           strerror(errno));
        exit (0);
   }

   /* This requirers linux-0.99.pl10 or above */
   *addr = (void *)mmap(NULL, *bank, PROT_READ|PROT_WRITE,
                             MAP_SHARED, fd, (off_t)offset);
   if (pid = fork()) {
        Display *disp;
	waitpid(pid, &status, 0);
	disp = XOpenDisplay(NULL);
	XF86VidModeDirectVideo(disp, screen, False);
	XAutoRepeatOn(dis);
	XSync(disp,False);
	_exit(0);
   }
}
