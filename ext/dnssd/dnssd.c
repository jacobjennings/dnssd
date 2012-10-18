#include "dnssd.h"
void Init_DNSSD_Errors(void);
void Init_DNSSD_Flags(void);
void Init_DNSSD_Record(void);
void Init_DNSSD_Service(void);

#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#include <Iphlpapi.h>
	#include <process.h>
	#define IF_NAMESIZE 256
	typedef int        pid_t;
	#define getpid     _getpid
	#define strcasecmp _stricmp
	#define snprintf   _snprintf
	static const char kFilePathSep = '\\';
	#ifndef HeapEnableTerminationOnCorruption
	#     define HeapEnableTerminationOnCorruption (HEAP_INFORMATION_CLASS)1
	#endif
	#if !defined(IFNAMSIZ)
	 #define IFNAMSIZ 16
    #endif
	#define if_nametoindex if_nametoindex_win
	#define if_indextoname if_indextoname_win

	typedef PCHAR (WINAPI * if_indextoname_funcptr_t)(ULONG index, PCHAR name);
	typedef ULONG (WINAPI * if_nametoindex_funcptr_t)(PCSTR name);

	unsigned if_nametoindex_win(const char *ifname)
		{
		HMODULE library;
		unsigned index = 0;

		// Try and load the IP helper library dll
		if ((library = LoadLibrary(TEXT("Iphlpapi")) ) != NULL )
			{
			if_nametoindex_funcptr_t if_nametoindex_funcptr;

			// On Vista and above there is a Posix like implementation of if_nametoindex
			if ((if_nametoindex_funcptr = (if_nametoindex_funcptr_t) GetProcAddress(library, "if_nametoindex")) != NULL )
				{
				index = if_nametoindex_funcptr(ifname);
				}

			FreeLibrary(library);
			}

		return index;
		}

	char * if_indextoname_win( unsigned ifindex, char *ifname)
		{
		HMODULE library;
		char * name = NULL;

		// Try and load the IP helper library dll
		if ((library = LoadLibrary(TEXT("Iphlpapi")) ) != NULL )
			{
			if_indextoname_funcptr_t if_indextoname_funcptr;

			// On Vista and above there is a Posix like implementation of if_indextoname
			if ((if_indextoname_funcptr = (if_indextoname_funcptr_t) GetProcAddress(library, "if_indextoname")) != NULL )
				{
				name = if_indextoname_funcptr(ifindex, ifname);
				}

			FreeLibrary(library);
			}

		return name;
		}

	static size_t _sa_len(const struct sockaddr *addr)
		{
		if (addr->sa_family == AF_INET) return (sizeof(struct sockaddr_in));
		else if (addr->sa_family == AF_INET6) return (sizeof(struct sockaddr_in6));
		else return (sizeof(struct sockaddr));
		}

#define SA_LEN(addr) (_sa_len(addr))
#endif

/*
 * call-seq:
 *   DNSSD.getservbyport(port, proto = nil) => service_name
 * 
 * Wrapper for getservbyport(3) - returns the service name for +port+.
 *
 *   DNSSD.getservbyport 1025 # => 'blackjack'
 */

static VALUE
dnssd_getservbyport(int argc, VALUE * argv, VALUE self) {
  VALUE _port, _proto;
  struct servent * result;
  int port;
  char * proto = NULL;

  rb_scan_args(argc, argv, "11", &_port, &_proto);

  port = htons(FIX2INT(_port));

  if (RTEST(_proto))
    proto = StringValueCStr(_proto);

  result = getservbyport(port, proto);

  if (result == NULL)
    return Qnil;

  return rb_str_new2(result->s_name);
}

/*
 * call-seq:
 *   DNSSD.interface_name(interface_index) # => interface_name
 *
 * Returns the interface name for interface +interface_index+.
 *
 *   DNSSD.interface_name 1 # => 'lo0'
 */

static VALUE
dnssd_if_nametoindex(VALUE self, VALUE name) {
  return UINT2NUM(if_nametoindex(StringValueCStr(name)));
}

/*
 * call-seq:
 *   DNSSD.interface_index(interface_name) # => interface_index
 *
 * Returns the interface index for interface +interface_name+.
 *
 *   DNSSD.interface_index 'lo0' # => 1
 */

static VALUE
dnssd_if_indextoname(VALUE self, VALUE index) {
  char buffer[IF_NAMESIZE];

  if (if_indextoname(NUM2UINT(index), buffer))
    return rb_str_new2(buffer);

  rb_raise(rb_eArgError, "invalid interface %d", NUM2UINT(index));

  return Qnil;
}

void
Init_dnssd(void) {

  VALUE mDNSSD = rb_define_module("DNSSD");

  /* All interfaces */
  rb_define_const(mDNSSD, "InterfaceAny",
      ULONG2NUM(kDNSServiceInterfaceIndexAny));

  /* Local interfaces, for services running only on the same machine */
  rb_define_const(mDNSSD, "InterfaceLocalOnly",
      ULONG2NUM(kDNSServiceInterfaceIndexLocalOnly));

#ifdef kDNSServiceInterfaceIndexUnicast
  /* Unicast interfaces */
  rb_define_const(mDNSSD, "InterfaceUnicast",
      ULONG2NUM(kDNSServiceInterfaceIndexUnicast));
#endif

  rb_define_singleton_method(mDNSSD, "getservbyport", dnssd_getservbyport, -1);

  rb_define_singleton_method(mDNSSD, "interface_index", dnssd_if_nametoindex, 1);
  rb_define_singleton_method(mDNSSD, "interface_name", dnssd_if_indextoname, 1);

  Init_DNSSD_Errors();
  Init_DNSSD_Flags();
  Init_DNSSD_Record();
  Init_DNSSD_Service();
}

