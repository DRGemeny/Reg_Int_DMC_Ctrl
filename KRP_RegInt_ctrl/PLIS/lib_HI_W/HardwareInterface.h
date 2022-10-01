#ifndef _HARDWARE_INTERFACE_H
#define _HARDWARE_INTERFACE_H

//константы кодов сегментов памяти КРП (для функций работы с КРП)
const int scOperSeg=0x0020;
const int scParamSeg=0x0021;
const int scDataSeg=0x0022;
const int scCycleSeg=0x0023;
const int riCOCmdReg=2;
const int riCODataReg=3;

/* Under Windows avoid including <windows.h> is overrated. */
#ifdef WIN32
#	include <windows.h>
#endif

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>


//#define MAX_PATH 0x100



#ifdef _UNICODE
#define _tcslen wcslen 
#else
#define _tcslen strlen
#endif

//#ifdef _UNICODE
//typedef wchar_t TCHAR;
//#else
//typedef char TCHAR;
//#endif




/* Macros used to declare C-linkage types and symbols. */
#ifdef __cplusplus
#	define HI_BEGIN_C_DECLS	extern "C" {
#	define HI_END_C_DECLS	}
#else
#	define HI_BEGIN_C_DECLS
#	define HI_END_C_DECLS
#endif

/* Platform-specific conditional compilation. */
#ifdef WIN32
	/*
	 * The following definitions control how symbols are exported.
	 *
	 * If the target is a static library ensure that HI_LIBRARY_STATIC
	 * is defined.
	 *
	 * If building a dynamic library (i.e. DLL) ensure the HI_LIBRARY
	 * macro is defined, as it will mark symbols for export.
	 *
	 * If compiling a project to _use_ the _dynamic_ library version of the library,
	 * no definition is required.
	 */
#	if defined(HI_LIBRARY_STATIC)	/* Static lib - no special export required. */
#		define HI_EXPORT
#	elif defined(HI_LIBRARY)	/* Dynamic lib - must export/import symbols appropriately. */
#		define HI_EXPORT	__declspec(dllexport)
#	else
#		define HI_EXPORT
#	endif

#else
	/*
	 * Ensure that the export symbol is defined (and blank).
	 */
#	define HI_EXPORT
#endif

#ifndef APIENTRY
#	define APIENTRY
#endif

HI_BEGIN_C_DECLS

/** Коды ошибок. */
typedef enum {
	brSuccess,
	brFailed,
	brTimeout,
	brDriverError,
	brMemLockError,
	brNotImplemented,
	brMemAllocError,
	brWaitError,
	brBadFileContent,
	brBadHandle,
	brBadAddress,
	brNoSuchEntry,
	brInvalidArgument,

	brCount
} ErrCode;

/** Идентификатор компонента "библиотека". */
#define HI_LIBRARY_COMPONENT_ID	(0x10000)
/** Идентификатор компонента "драйвер". */
#define HI_DRIVER_COMPONENT_ID	(0x20000)
 
/** Структура элемента списка версий компонент. */
typedef struct _hi_version_list_item {
	/** Идентификатор компонента. */
	unsigned int  id;
	/** Длина данных. */
	size_t  size;
	/** Данные компонента. */
	const void  *data;
} hi_version_list_item_t;
 
/** Структура списка элементов версий компонент. */
typedef struct _hi_version_list {
	/** Количество элементов списка. */
	unsigned int  count;
	/** Элементы списка. */
	const hi_version_list_item_t  *item;
} hi_version_list_t;
 
/** Структура версии Mercurial компонента. */
typedef struct _hi_component_mercurial_version {
	/** Полная версия компонента. */
	uint64_t  changeset;
	/** Короткая версия компонента. */
	unsigned long  changeset_short;
} hi_component_mercurial_version_t;

extern int g_Emulation;
extern int g_Debug;

HI_EXPORT int APIENTRY OpenInterface(const char *ip, void **handle);
HI_EXPORT int APIENTRY CloseInterface(void *handle);

HI_EXPORT int APIENTRY WriteCOReg(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int value);
HI_EXPORT int APIENTRY WriteCOCmd(void *handle, unsigned int bm_id, unsigned int value);

HI_EXPORT int APIENTRY ReadCOReg(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int *value);
HI_EXPORT int APIENTRY ReadCOCmd(void *handle, unsigned int bm_id, unsigned int *value);

HI_EXPORT int APIENTRY WriteBufferCOReg(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int length, const unsigned int *buffer);
HI_EXPORT int APIENTRY WriteBufferCODataReg(void *handle, unsigned int bm_id, unsigned int length, const unsigned int *buffer);

HI_EXPORT int APIENTRY ReadBufferCOReg(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int length, unsigned int *buffer);
HI_EXPORT int APIENTRY ReadBufferCODataReg(void *handle, unsigned int bm_id, unsigned int length, unsigned int *buffer);

HI_EXPORT int APIENTRY ReadCOMem32(void *handle, unsigned int bm_id, unsigned int length, unsigned int *buffer);
HI_EXPORT int APIENTRY WriteCOMem32(void *handle, unsigned int bm_id, unsigned int length, const unsigned int *buffer);

HI_EXPORT int APIENTRY ReadMemFast(void *handle, unsigned int bm_id, unsigned int code, unsigned int dmc_id, unsigned int addr, unsigned int step, unsigned int length, unsigned int *buffer);
HI_EXPORT int APIENTRY WriteMemFast(void *handle, unsigned int bm_id, unsigned int code, unsigned int dmc_id, unsigned int addr, unsigned int step, unsigned int length, const unsigned int *buffer);

HI_EXPORT int APIENTRY ReadMem64Fast(void *handle, unsigned int bm_id, unsigned int code, unsigned int dmc_id, unsigned int addr, unsigned int step, unsigned int length, uint64_t *buffer);
HI_EXPORT int APIENTRY WriteMem64Fast(void *handle, unsigned int bm_id, unsigned int code, unsigned int dmc_id, unsigned int addr, unsigned int step, unsigned int length, const uint64_t *buffer);

HI_EXPORT int APIENTRY ReadMem64(void *handle, unsigned int bm_id, unsigned int code, unsigned int dmc_id, unsigned int addr, unsigned int step, unsigned int length, uint64_t *buffer);
HI_EXPORT int APIENTRY WriteMem64(void *handle, unsigned int bm_id, unsigned int code, unsigned int dmc_id, unsigned int addr, unsigned int step, unsigned int length, const uint64_t *buffer);

HI_EXPORT int APIENTRY ReadDMCReg(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int dmc_id, unsigned int *value);
HI_EXPORT int APIENTRY WriteDMCReg(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int dmc_id, unsigned int value);

HI_EXPORT int APIENTRY ReadDMCReg64(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int dmc_id, uint64_t *value);
HI_EXPORT int APIENTRY WriteDMCReg64(void *handle, unsigned int bm_id, unsigned int reg_code, unsigned int dmc_id, uint64_t value);

HI_EXPORT int APIENTRY LoadFileFast(void *handle, unsigned int bm_id, const char *filename);
HI_EXPORT int APIENTRY LoadFile64Fast(void *handle, unsigned int bm_id, const char *filename);

HI_EXPORT int APIENTRY GlobalReset(void *handle, unsigned int bm_id);
HI_EXPORT int APIENTRY RunDMC(void *handle, unsigned int bm_id);
HI_EXPORT int APIENTRY StopDMC(void *handle, unsigned int bm_id);
HI_EXPORT int APIENTRY WaitEndStatus(void *handle, unsigned int bm_id, unsigned long *time, unsigned long timeout);

HI_EXPORT int APIENTRY SpecBuffAlloc(void *handle, unsigned int length, uint64_t **pbuf);
HI_EXPORT int APIENTRY SpecBuffFree(void *handle, unsigned int length, uint64_t *buf);

HI_EXPORT int APIENTRY ReadBAR3(void *handle, unsigned int index, unsigned short *value);
HI_EXPORT int APIENTRY WriteBAR3(void *handle, unsigned int index, unsigned short value);

HI_EXPORT int APIENTRY GetVer(void *handle, const void **buffer);

HI_END_C_DECLS


#endif /* _HARDWARE_INTERFACE_H */

