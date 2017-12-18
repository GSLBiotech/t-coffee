#include "platform_lib.h"

#ifdef _MSC_VER
#include <tlhelp32.h>
// gethostname
#include <Winsock2.h>

//=========================================================
int access(const char* path, int /* modeUnused */)
{
  //MSDN: Windows 2000 and later operating systems, all directories have read and write access.
  return _access( path, 0 );
}
//=========================================================
int tc_mkdir(const char *path, int /* permissionUnused */)
{
  return mkdir( path );
}
//=========================================================
void tc_gethostname(char *name, int namelen)
{
  if( !name ||
      namelen <= 1)
    return;

  static char cache[MAX_PATH] = {0};
  if( !cache[0] )
  {
    WSADATA wsaData;
    WSAStartup( MAKEWORD(2, 2), & wsaData );
    gethostname( cache, MAX_PATH );
    WSACleanup();
  }

  strncpy( name, cache, namelen < MAX_PATH ? namelen : MAX_PATH );
  // Ensure extremely long string is valid null terminated, bug in c lib.
  name[namelen-1] = '\0';
}
//=========================================================
pid_t getppid()
{
  HANDLE h = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  PROCESSENTRY32 pe = { 0 };
  pe.dwSize = sizeof( PROCESSENTRY32) ;
  int pid = GetCurrentProcessId();
  int result = -1;

  if( Process32First( h, & pe ) )
  {
    do
    {
      if( pe.th32ProcessID == pid )
      {
        result = pe.th32ParentProcessID;
        break;
      }
    } while( Process32Next( h, & pe ) );
  }

  CloseHandle( h );

  return result;
}
//=========================================================
void kill(pid_t pid, int exitCode)
{
  HANDLE h = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
  BOOL result = TerminateProcess( h, exitCode );
  CloseHandle( h );
}
//=========================================================
pid_t wait(int* status)
{
  return waitpid( -1, status, 0 );
}
//=========================================================
pid_t waitpid(pid_t pid, int* status, int /* optionsUnused */)
{
  if( status )
  {
    *status = -1;
  }

  if( -1 == pid )
  {
    pid = getpid();
  }

  bool success = false;
  HANDLE h = OpenProcess( PROCESS_ALL_ACCESS, FALSE, pid );
  DWORD result = WaitForSingleObject( h, 60000 ); //milliseconds
  switch( result )
  {
    case WAIT_OBJECT_0:
      success = true;
      break;
    case WAIT_TIMEOUT:
    case WAIT_ABANDONED:
    case WAIT_FAILED:
    default:
      success = TerminateProcess( h, 1 );
      break;
  }

  return success ? pid : -1; // Error
}
//=========================================================
#endif
