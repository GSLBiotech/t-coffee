#ifndef PLATFORM_LIB_H_
#define PLATFORM_LIB_H_

#ifdef _MSC_VER
  #define ENV_HOME "USERPROFILE"
  #define WIN32_LEAN_AND_MEAN

  // CreateDirectory
  #include <Windows.h>

  #include <direct.h>
  #define _chdir  chdir
  #define _getcwd getcwd

  // FindFirstFile
  #include <Windows.h>
  #define popen(CMD,MODE) _popen(CMD,MODE)

  // _mktemp
  #include <io.h>
  int access(const char* path, int /* modeUnused */);
  //Arbitrary, unused for access().
  #define F_OK 0
  #define W_OK 0
  #define R_OK 0
  #define X_OK 0
  int tc_mkdir(const char* path, int /* permissionUnused */);

  void tc_gethostname(char *name, int namelen);

  #include <process.h>
  #define getpid _getpid
  #define P_WAIT _P_WAIT
  #define spawnv _spawnv
  typedef int pid_t;
  pid_t getppid();
  void kill(pid_t p, int /* signalUnused */ );
  //Wait for child process to terminate. Returns terminated child pid, or -1 on error.
  pid_t wait(int* status);
  pid_t waitpid(pid_t pid, int* status, int /* optionsUnused */);
#else
  #define ENV_HOME "HOME"

  #include <dirent.h>
  #include <sched.h>
  #include <strings.h>
  #include <sys/file.h>
  #include <sys/times.h>
  #include <sys/wait.h>
  #include <unistd.h>

  #define mkdir tc_mkdir
  #define gethostname tc_gethostname
#endif

#endif // PLATFORM_LIB_H_