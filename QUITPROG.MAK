# IBM Developer's Workframe/2 Make File Creation run at 21:27:49 on 09/06/97

# Make File Creation run in directory:
#   F:\PROJS\QUITPROG;

.SUFFIXES:

.SUFFIXES: .c .cpp .cxx

QUITPROG.EXE:  \
  QuitProg.OBJ \
  QUITPROG.MAK
   ICC.EXE @<<
 /Fe"QuitProg.exe" 
QuitProg.OBJ
<<

{.}.c.obj:
   ICC.EXE /Q /Fi /Si /C   .\$*.c

{.}.cpp.obj:
   ICC.EXE /Q /Fi /Si /C   .\$*.cpp

{.}.cxx.obj:
   ICC.EXE /Q /Fi /Si /C   .\$*.cxx

!include QUITPROG.DEP
