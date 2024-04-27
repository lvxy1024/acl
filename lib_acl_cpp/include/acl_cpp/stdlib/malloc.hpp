#pragma once
#include "../acl_cpp_define.hpp"

#ifdef ACL_HOOK_NEW

#ifdef ACL_CPP_DEBUG_MEM
#undef ACL_CPP_DEBUG_MEM
#endif

# if defined(_WIN32) || defined(_WIN64)
	#  ifdef NDEBUG
		#include <new>
		void* operator new(size_t n);
		void  operator delete(void *p);
	#  endif
# else
	#include <new>

	void* operator new(std::size_t);                                      // throw(std::bad_alloc)
	void* operator new[](std::size_t);                                    // throw(std::bad_alloc)
	void* operator new(std::size_t, const std::nothrow_t&) noexcept;      // return nullptr
	void* operator new[](std::size_t, const std::nothrow_t&) noexcept;    // return nullptr

	void operator delete(void*) noexcept;
	void operator delete[](void*) noexcept;
	void operator delete(void*, std::size_t) noexcept;
	void operator delete[](void*, std::size_t) noexcept;

	void operator delete(void*, const std::nothrow_t&) noexcept;                   // �ڲ��׳��쳣��new���ʽ�У��������Ĺ��캯���׳��쳣ʱ�ᱻ����
	void operator delete[](void*, const std::nothrow_t&) noexcept;                 // �ڲ��׳��쳣��new[]���ʽ�У������һ������Ĺ��캯���׳��쳣ʱ�ᱻ����
	void operator delete(void*, std::size_t, const std::nothrow_t&) noexcept;      // �ڲ��׳��쳣��new���ʽ�У��������Ĺ��캯���׳��쳣ʱ�ᱻ����
	void operator delete[](void*, std::size_t, const std::nothrow_t&) noexcept;    // �ڲ��׳��쳣��new[]���ʽ�У������һ������Ĺ��캯���׳��쳣ʱ�ᱻ����
# endif

#elif defined(ACL_CPP_DEBUG_MEM)

/**
 * ����Ҫ��� acl ���е��ڴ�������Ƿ�����ڴ�й¶����ʱ��ͨ������ new/delete
 * ������Ĵ��������ٹ���¼������������󱻴�����û�б��ͷţ����ڲ��ͻ���Ϊ
 * һ���ڴ�й¶��ʹ���߿��Զ�ʱ�����Щ��ַ��Ϣ���鿴���ĸ��ļ������Ķ���û��
 * ���ͷţ�Ҫ��ʹ�ô˹��ܣ����� lib_acl_cpp/include/acl_cpp/acl_cpp_define.hpp
 * �еĺ� "//#define ACL_CPP_DEBUG_MEM" �򿪣�ͬʱӦ����Ҫ���Լ���Դ�����а���
 * ͷ�ļ���
 * #include "acl_cpp/stdlib/malloc.hpp
 * ���Ӧ�ó����ڴ�������ʱӦʹ�� NEW ����� new.
 */

#include <new>

void* operator new(std::size_t, const char*, const char*, int);                                      // throw(std::bad_alloc)
void* operator new[](std::size_t, const char*, const char*, int);                                    // throw(std::bad_alloc)
void* operator new(std::size_t, const std::nothrow_t&, const char*, const char*, int) noexcept;      // return nullptr
void* operator new[](std::size_t, const std::nothrow_t&, const char*, const char*, int) noexcept;    // return nullptr

// ��ʾ����
void operator delete(void*) noexcept;
void operator delete[](void*) noexcept;
void operator delete(void*, std::size_t) noexcept;
void operator delete[](void*, std::size_t) noexcept;

// ��ʽ����
void operator delete(void*, const std::nothrow_t&) noexcept;                   // �ڲ��׳��쳣��new���ʽ�У��������Ĺ��캯���׳��쳣ʱ�ᱻ����
void operator delete[](void*, const std::nothrow_t&) noexcept;                 // �ڲ��׳��쳣��new[]���ʽ�У������һ������Ĺ��캯���׳��쳣ʱ�ᱻ����
void operator delete(void*, std::size_t, const std::nothrow_t&) noexcept;      // �ڲ��׳��쳣��new���ʽ�У��������Ĺ��캯���׳��쳣ʱ�ᱻ����
void operator delete[](void*, std::size_t, const std::nothrow_t&) noexcept;    // �ڲ��׳��쳣��new[]���ʽ�У������һ������Ĺ��캯���׳��쳣ʱ�ᱻ����

void operator delete(void*, const char*, const char*, int) noexcept;                         // �����캯���׳��쳣ʱ�ᱻӵ��ƥ��ǩ�����Զ��岼��new���ʽ����
void operator delete[](void*, const char*, const char*, int) noexcept;                       // �����캯���׳��쳣ʱ�ᱻӵ��ƥ��ǩ�����Զ��岼��new[]���ʽ����
void operator delete(void*, std::size_t, const char*, const char*, int) noexcept;
void operator delete[](void*, std::size_t, const char*, const char*, int) noexcept;

void operator delete(void*, const std::nothrow_t&, const char*, const char*, int) noexcept;
void operator delete[](void*, const std::nothrow_t&, const char*, const char*, int) noexcept;
void operator delete(void*, std::size_t, const std::nothrow_t&, const char*, const char*, int) noexcept;
void operator delete[](void*, std::size_t, const std::nothrow_t&, const char*, const char*, int) noexcept;

#define NEW new(__FILE__, __FUNCTION__, __LINE__)

// #define NEW new(__FILE__, __FUNCTION__, __LINE__)
// #define NEW_ARGS(...) new(__VA_ARGS__, __FILE__, __FUNCTION__, __LINE__)

namespace acl {

/**
 * �����ڴ��ع���
 */
ACL_CPP_API void mem_checker_start(const char* logfile = NULL);

/**
 * ����ǰ�ڴ�״̬��ʾ����Ļ��ʹ���߿����Լ����Ӷ�ʱ����ʱ���ô˺���
 */
ACL_CPP_API void mem_checker_show(void);
}

#endif

namespace acl {

ACL_CPP_API void  acl_slice_init(void);

/**
 * �ڴ���亯��
 * @param size {size_t} ��Ҫ����ĳߴ��С
 * @param filename {const char*} Դ��������
 * @param funcname {const char*} ������
 * @param lineno {int} Դ�����к�
 * @return {void*} ������ڴ��ַ����������ڴ�ʧ�ܣ���ֱ��abort
 */
ACL_CPP_API void* acl_new(size_t size, const char* filename,
	const char* funcname, int lineno);


/**
 * �ͷ��ڴ溯��
 * @param ptr {void*} �ڴ��ַ
 * @param filename {const char*} Դ��������
 * @param funcname {const char*} ������
 * @param lineno {int} Դ�����к�
 */
ACL_CPP_API void  acl_delete(void *ptr, const char* filename,
	const char* funcname, int lineno);

}
