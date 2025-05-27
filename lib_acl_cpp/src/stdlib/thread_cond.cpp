#include "acl_stdafx.hpp"
#ifndef ACL_PREPARE_COMPILE
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stdlib/util.hpp"
#include "acl_cpp/stdlib/thread_mutex.hpp"
#include "acl_cpp/stdlib/thread_cond.hpp"
#endif

namespace acl {

thread_cond::thread_cond(thread_mutex* mutex)
{
	if (mutex) {
		mutex_internal_ = NULL;
		mutex_ = mutex;
	} else {
		mutex_internal_ = NEW thread_mutex;
		mutex_  = mutex_internal_;
	}

	cond_ = (acl_pthread_cond_t*)
		acl_mycalloc(1, sizeof(acl_pthread_cond_t));
	acl_pthread_cond_init(cond_, NULL);
}

thread_cond::~thread_cond()
{
	acl_pthread_cond_destroy(cond_);
	acl_myfree(cond_);
	delete mutex_internal_;
}

bool thread_cond::notify()
{
	return acl_pthread_cond_signal(cond_) == 0;
}

bool thread_cond::notify_all()
{
	return acl_pthread_cond_broadcast(cond_) == 0;
}

#define	SEC_TO_NS	1000000000	// nanoseconds per second
#define SEC_TO_US	1000000		// microseconds per second
#define US_TO_NS	1000		// nanoseconds per microseconds

bool thread_cond::wait(long long microseconds /* = -1 */,
	bool locked /* = false */)
{
	if (microseconds >= 0) {
		return timed_wait(microseconds, locked);
	} else {
		return block_wait(locked);
	}
}

bool thread_cond::block_wait(bool locked)
{
	bool locked_internal;

	if (!locked) {
		if (!mutex_->lock()) {
			logger_error("lock error=%s", last_serror());
			return false;
		}
		locked_internal = true;
	} else {
		locked_internal = false;
	}

	int ret = acl_pthread_cond_wait(cond_, mutex_->get_mutex());
	if (ret) {
#ifdef ACL_UNIX
		acl_set_error(ret);
#endif
		logger_error("pthread_cond_wait error %s", last_serror());
	}

	// ����������ڲ�ǰ�����������˴���Ҫ����
	if (locked_internal && !mutex_->unlock()) {
		logger_error("mutex unlock error=%s", last_serror());
		return false;
	}

	return ret == 0 ? true : false;
}

bool thread_cond::timed_wait(long long microseconds, bool locked)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	long long ns = (tv.tv_usec + microseconds % SEC_TO_US) * US_TO_NS;
	// Sanity check.
	if (ns < 0) {
		ns = 0;
	}

	struct timespec ts;
	ts.tv_sec    = (long) (tv.tv_sec + microseconds / SEC_TO_US);
	ts.tv_sec   += (long) (ns / SEC_TO_NS);
	ts.tv_nsec   = (long) (ns % SEC_TO_NS);

	bool locked_internal;
	if (mutex_internal_ || !locked) {
		if (!mutex_->lock()) {
			logger_error("lock error=%s", last_serror());
			return false;
		}
		locked_internal = true;
	} else {
		locked_internal = false;
	}

	int ret = acl_pthread_cond_timedwait(cond_, mutex_->get_mutex(), &ts);
	if (ret) {
#ifdef ACL_UNIX
		acl_set_error(ret);
#endif
		if (ret != ACL_ETIMEDOUT) {
			logger_error("pthread_cond_timedwait error=%s",
				last_serror());
		}
	}

	if (locked_internal && !mutex_->unlock()) {
		logger_error("mutex unlock error=%s", last_serror());
		return false;
	}

	return ret == 0;
}

thread_mutex& thread_cond::get_mutex() const
{
	return *mutex_;
}

acl_pthread_cond_t* thread_cond::get_cond() const
{
	return cond_;
}

} // namespace acl
