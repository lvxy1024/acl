#pragma once
#include "../acl_cpp_define.hpp"
#include <cassert>
#include "box.hpp"

namespace acl {

// internal functions being used
void*  mbox_create(bool mpsc);
void   mbox_free(void*, void (*free_fn)(void*));
bool   mbox_send(void*, void*);
void*  mbox_read(void*, int, bool*);
size_t mbox_nsend(void*);
size_t mbox_nread(void*);

/**
 * ���������߳�֮�䡢Э��֮�����ͨ�ŵ��࣬�ڲ�ʵ�ֲ����������� + IO ͨ����
 * ��Ϸ�ʽʵ��
 *
 * ʾ��:
 *
 * class myobj {
 * public:
 *     myobj() {}
 *     ~myobj() {}
 *     
 *     void run()
 *     {
 *         printf("hello world!\r\n");
 *     }
 * };
 * 
 * acl::mbox<myobj> mbox;
 *
 * void thread_producer()
 * {
 *     myobj* o = new myobj;
 *     mbox.push(o);
 * }
 * 
 * void thread_consumer()
 * {
 *     myobj* o = mbox.pop();
 *     o->run();
 *     delete o;
 * }
 */

template<typename T>
class mbox : public box<T> {
public:
	/**
	 * ���췽��
	 * @param free_obj {bool} �� tbox ����ʱ���Ƿ��Զ���鲢�ͷ�
	 *  δ�����ѵĶ�̬����
	 * @param mpsc {bool} �Ƿ�Ϊ��������-��������ģʽ
	 */
	explicit mbox(bool free_obj = true, bool mpsc = true)
	: free_obj_(free_obj)
	{
		mbox_ = mbox_create(mpsc);
		assert(mbox_);
	}

	~mbox() {
		mbox_free(mbox_, free_obj_ ? mbox_free_fn : NULL);
	}

	/**
	 * ������Ϣ����
	 * @param t {T*} �ǿ���Ϣ����
	 * @return {bool} �����Ƿ�ɹ�
	 * @override
	 */
	bool push(T* t, bool dummy = false) {
        (void) dummy;
		return mbox_send(mbox_, t);
	}

	/**
	 * ������Ϣ����
	 * @param ms {int} >= 0 ʱ���ö��ȴ���ʱʱ��(���뼶��)������
	 *  ��Զ�ȴ�ֱ��������Ϣ��������
	 * @param success {bool*} �������ڸ���ȷ���������Ƿ�ɹ�
	 * @return {T*} �� NULL ��ʾ����һ����Ϣ����Ϊ NULL ʱ������ͨ��
	 *  success �����ķ���ֵ�������Ƿ�ɹ�
	 * @override
	 */
	T* pop(int ms = -1, bool* success = NULL) {
		return (T*) mbox_read(mbox_, ms, success);
	}

	// @override
	size_t pop(std::vector<T*>& out, size_t max, int ms) {
		size_t n = 0;
		bool success;
		while (true) {
			T* t = (T*) mbox_read(mbox_, ms, &success);
			if (! t) {
				return n;
			}

			out.push_back(t);
			n++;
			if (max > 0 && n >= max) {
				return n;
			}
			ms = 0;
		}
	}

	/**
	 * mbox ��֧�ִ��ݿ���Ϣ
	 * @return {bool}
	 * @override
	 */
	bool has_null() const {
		return false;
	}

	/**
	 * ͳ�Ƶ�ǰ�Ѿ����͵���Ϣ��
	 * @return {size_t}
	 */
	size_t push_count() const {
		return mbox_nsend(mbox_);
	}

	/**
	 * ͳ�Ƶ�ǰ�Ѿ����յ�����Ϣ��
	 * @return {size_t}
	 */
	size_t pop_count() const {
		return mbox_nread(mbox_);
	}

private:
	void* mbox_;
	bool  free_obj_;

	static void mbox_free_fn(void* o) {
		T* t = (T*) o;
		delete t;
	}
};

} // namespace acl
