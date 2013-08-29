/******************************** Module Head ***************************
* Author      : Jõrgen Thomsen (j.thomsen@usa.net)                      *
*               (Please send me an email, if you use this program)      *
* File/Modul  : QuitProg.C                                              *
* Program type: OS/2 command line program                               *
* Short descr.: Makes OS/2 PM program save data and/or close and exit   *
* Keywords    : OS/2, CLOSE, EXIT, TERMINATE, SAVE, WM_QUIT, PROGRAM    *
* Description : Will send a WM_SAVEAPLLICATION and/or a WM_QUIT message *
*               to all tasks in the task list containing a certain      *
*               string in the title.                                    *
* Language    : IBM C Set++ version 2.0                                 *
* INPUT       : command line parameters                                 * 
*               <search string> string to be found in title, not case   *
*                               sensitive EPM or "big program"          *
*               <msgtype>       S, Q, SQ for save data, quit program or *
*                               both. Default: Q                        *
*               <timeout>       seconds, wait for specified time for    *
*                               programs to terminate. Default: 60      *
*               e.g. QuitProg EPM Q 10                                  *
* OUTPUT:       message for each command sent to each program on stdout *
*               non-zero return code in case of error or timeout        *
* ------------+----------------+--------------------------------------- *
* Revision    | Date     / ID  | Changes                                *
* ------------+----------------+--------------------------------------- *
*   1.00      | 05-09-97 / JùT | Initial release                        *
*************************************************************************/
#define INCL_NOCOMMON
#define INCL_DOSPROCESS   /* Process and thread values */

#define INCL_WINWINDOWMGR
#define INCL_WINMESSAGEMGR
#define INCL_WINERRORS
#define INCL_WINSWITCHLIST

#include <os2.h>
#include <string.h>
#include <stdio.h>

#ifdef XPQ /* informational only */
      typedef struct _SWCNTRL          /* swctl */
      {
         HWND     hwnd;
         HWND     hwndIcon;
         HPROGRAM hprog;
         PID      idProcess;
         ULONG    idSession;
         ULONG    uchVisibility;
         ULONG    fbJump;
         CHAR     szSwtitle[MAXNAMEL+4];
         ULONG    bProgType;
      } SWCNTRL;
      
      typedef SWCNTRL *PSWCNTRL;
            
      typedef struct _SWENTRY          /* swent */
      {
         HSWITCH hswitch;
         SWCNTRL swctl;
      } SWENTRY;
      typedef SWENTRY *PSWENTRY;
      
      typedef struct _SWBLOCK          /* swblk */
      {
         ULONG    cswentry;
         SWENTRY aswentry[1];
      } SWBLOCK;

      typedef SWBLOCK *PSWBLOCK;
 
#endif

#define MAXBUF 32000
#define DELAYVAL 500  /* ms */

CHAR    buf[MAXBUF];
APIRET  rc = 0;
USHORT  res = 0;
CHAR    buf1[MAXNAMEL+4];

void main (int argc, char *argv[])
{
 int      i = 0, cnt = 0;
 USHORT   res = 2;
 CHAR     *pstr = NULL;

 HAB      hab = NULLHANDLE;                  /* PM anchor block handle         */
 HMQ      hmq = NULLHANDLE;                  /* message queue handle           */

 ULONG    cbItems = 0, il;
 LONG     timout = 60;
 PVOID    pBase = NULL;
 PSWBLOCK pswblk = NULL, pswblk1 = NULL;
 SWCNTRL  *pswcntrl = NULL;
 ERRORID  err = 0;
 PVOID    SaveAction = NULL, QuitAction = NULL;

 if (argc < 2 || argc > 4) /* 1 parameter required */
 { 
    printf("Syntax: %s <Search string> [ [SQ|S|Q] [<timeout sec. (def. 60)>]]\n", argv[0]);
    exit(1);
   }
   pstr = argv[1];
   strlwr(argv[1]);
   if (argc == 4) { timout = atoi(argv[3]);} 
   timout = timout * 1000 / DELAYVAL;
   if (argc >= 3) { 
      strupr(argv[2]); 
      SaveAction = strchr(argv[2], 'S');
      QuitAction = strchr(argv[2], 'Q'); } /* endif */
   if (QuitAction == NULL) {timout = 0;} /* endif */

   hab = WinInitialize( 0UL );
   hmq = WinCreateMsgQueue( hab, 0UL ); /* not used and created only if compiled as PM program */

/*
   This code calls WinQuerySwitchList to fill the buffer with the information about
   each program in the Task List.
*/

      pswblk = (PSWBLOCK) &buf[0];
      cbItems = WinQuerySwitchList(hab, pswblk, MAXBUF);       /* gets struct. array */

/* check each task */
      for (i=0; i < cbItems ; i++ ) {
         pswcntrl = &pswblk->aswentry[i].swctl;
         pstr = &pswblk->aswentry[i].swctl.szSwtitle[0];
         strcpy(buf1, pstr);  
         strlwr(buf1);
         if (strstr(buf1, argv[1])) { 
           res = 0; cnt++;
 
         if (SaveAction) {
            /* send WM_SAVEAPPLICATION message */
             printf("Sending save command to '%s'\n", pstr);
             WinPostMsg(pswcntrl->hwnd, WM_SAVEAPPLICATION,
                        MPFROM2SHORT(0, 0), MPFROM2SHORT(0, 0));

             err = WinGetLastError(hab) & 0xffff;
             if (err && err != PMERR_NOT_IN_A_PM_SESSION) {
               printf("Error on save: %x\n", err);
             }
         } /* endif */

         if (QuitAction) {
         /* send WM_QUIT message */
            printf("Sending quit command to '%s'\n", pstr);
            WinPostMsg(pswcntrl->hwnd, WM_QUIT,
                       MPFROM2SHORT(0, 0), MPFROM2SHORT(0, 0));
            err = WinGetLastError(hab) & 0xffff;
            if (err && err != PMERR_NOT_IN_A_PM_SESSION) {
              if (err == PMERR_INVALID_HWND ) { printf(" Invalid window handle\n");
              } else { 
                if (err == PMERR_WINDOW_NOT_LOCKED ){ printf(" Window not locked\n");
                } else {
                   printf("Error on quit: %x\n", err);
                } /* endif */
              }
            }
          } /* end if QuitAction */
         } /* end if strstr */
      } /* endfor */

      if (timout != 0) { /* quit specified, wait for progs to terminate */
        do {     
          cbItems = WinQuerySwitchList(hab, pswblk, MAXBUF);       /* gets struct. array */
          cnt = 0;
          for (i=0; i < cbItems ; i++ ) {
            pstr = &pswblk->aswentry[i].swctl.szSwtitle[0]; 
            strcpy(buf1, pstr); strlwr(buf1);
            if (strstr(buf1, argv[1])) { 
              cnt++;
              if (timout == 1) {printf("Timeout on '%s'. Not terminated!\n", pstr); res = 3;}
            } /* endif */
          } /* endfor */

          if (cnt > 0) {rc = DosSleep(DELAYVAL); timout--;} /* endif */
        } while (cnt > 0 && timout > 0 ); /* enddo */

      } /* end if timout */

    /* Destroy the possible message queue and release the anchor block. */

   if ( hmq != NULLHANDLE )
      WinDestroyMsgQueue( hmq );

   if ( hab != NULLHANDLE )
      WinTerminate( hab );

exit(res);
} /* end main */



