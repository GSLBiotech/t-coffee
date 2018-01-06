#ifndef PLATFORM_LIB_H_
#define PLATFORM_LIB_H_

#ifdef _MSC_VER
  #define ENV_HOME "USERPROFILE"
  #define WIN32_LEAN_AND_MEAN
  #define strtok_r strtok_s

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
  #include <spawn.h>

  #define tc_mkdir mkdir
  #define tc_gethostname gethostname
#endif

//=========================================================
//Thread-safe print of current thread id and a message. Useful for debugging.
#include <iostream>
#include <sstream>
#include <mutex>
#include <thread>

#define PRINT_THREAD()            PrintThread(__FILE__, __LINE__, __PRETTY_FUNCTION__);
#define PRINT_THREAD1(MSG)        PrintThread(__FILE__, __LINE__, __PRETTY_FUNCTION__, MSG);
#define PRINT_THREAD2(MSG, VALUE) PrintThread(__FILE__, __LINE__, __PRETTY_FUNCTION__, MSG, VALUE);

class PrintThread: public std::ostringstream
{
public:
  PrintThread() = default;

  PrintThread(const char* file, int line, const char* func)
  { *this << file << ':' << line << ' ' << func << ' '; }

  PrintThread(const char* file, int line, const char* func, const char* msg)
  { *this << file << ':' << line << ' ' << func << ' ' << msg; }

  template<typename T>
  PrintThread(const char* file, int line, const char* func, const char* msg, T value)
  { *this << file << ':' << line << ' ' << func << ' ' << msg << value; }

  ~PrintThread()
  {
    std::lock_guard<std::mutex> guard(_mutexPrint);
    std::cout << std::this_thread::get_id() << ' ' << this->str() << '\n';
  }

private:
  static std::mutex _mutexPrint;
};
//=========================================================

#endif // PLATFORM_LIB_H_
