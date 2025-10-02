#pragma once
#include "../acl_cpp_define.hpp"
#include <map>
#include <string>
#include "singleton.hpp"

#define	ACL_COUNTER_INIT(thread_safe)                                         \
	acl::class_counter::get_instance().init(thread_safe)

// ĳ���ַ��������һ��, flagName Ϊ�Լ������Ψһ�ַ�����
#define ACL_COUNTER_INC(flagName)                                             \
	acl::class_counter::get_instance().inc(#flagName)

// ĳ���ַ���Ǽ���һ��
#define ACL_COUNTER_DEC(flagName)                                             \
	acl::class_counter::get_instance().dec(#flagName)

#define	ACL_COUNTER_COUNT(flagName)                                           \
	acl::class_counter::get_instance().count(#flagName)

#define	ACL_COUNTER_PRINT()                                                   \
	acl::class_counter::get_instance().print()

namespace acl {

class thread_mutex;

class ACL_CPP_API class_counter : public singleton<class_counter> {
public:
	/**
	 * ���췽��
	 * @param clean {bool} �Ƿ��Զ��������Ϊ 0 �ļ�������.
	 */
	class_counter(bool clean = true);
	~class_counter();

	/**
	 * �����ڽ��̳�ʼ��ʱ���ñ�����(�Ǳ���)���г�ʼ��,ָ���ڲ��Ƿ���Ҫ����,
	 * ��������ñ�����,�ڲ��Զ����߳���.
	 */
	void init(bool thread_safe = true);

	/**
	 * @brief ��name��Ӧ�ļ�����1,��û��nameʱ����Ϊ1
	 */
	void inc(const char *name);

	/**
	 * @brief ��name��Ӧ�ļ�����1,��û��nameʱ�����������־
	 */
	void dec(const char *name);

	/**
	 * @brief ��ȡname�����ͳ�Ƹ���
	 */
	long long count(const char *name);

	/**
	 * @brief �������ͳ��
	 * @param flag �����߱��
	 */
	void print(const char *flag = NULL);

private:
	std::map<std::string, long long> names_;
	thread_mutex* lock_;
	bool clean_;
};

}  // namespace acl
