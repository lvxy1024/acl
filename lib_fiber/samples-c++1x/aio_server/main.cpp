#include "stdafx.h"
#include "lib_acl.h" // just for getopt on Windows
#include <cassert>
#include <iostream>

static int  s_max = 0;
static int  s_timeout = 0;
static bool s_use_kernel = false;
static bool s_use_reactor = false;

/**
 * �ӳٶ��ص�������
 */
class timer_reader: public acl::aio_timer_reader {
public:
	explicit timer_reader(long long delay) {
		delay_ = delay;
		std::cout << "timer_reader init, delay: " << delay << std::endl;
	}

protected:
	~timer_reader() override = default;

	// aio_timer_reader ������������� destroy ����
	// @override
	void destroy() override {
		std::cout << "timer_reader delete, delay: "  << delay_ << std::endl;
		delete this;
	}

	// ���ػ���ص�����
	// @override
	void timer_callback(unsigned int id) override {
		std::cout << "timer_reader(" << id
			<< "): timer_callback, delay: " << delay_ << std::endl;

		// ���û���Ĵ������
		aio_timer_reader::timer_callback(id);
	}

private:
	long long delay_;
};

/**
 * �ӳ�д�ص�������
 */
class timer_writer: public acl::aio_timer_writer {
public:
	explicit timer_writer(long long delay) {
		delay_ = delay;
		std::cout << "timer_writer init, delay: " << delay << std::endl;
	}

protected:
	~timer_writer() override = default;

	// aio_timer_reader ������������� destroy ����
	// @override
	void destroy() override{
		std::cout << "timer_writer delete, delay: " << delay_ << std::endl;
		delete this;
	}

	// ���ػ���ص�����
	// @override
	void timer_callback(unsigned int id) override {
		std::cout << "timer_writer(" << id << "): timer_callback, delay: "
			<< delay_ << std::endl;

		// ���û���Ĵ������
		acl::aio_timer_writer::timer_callback(id);
	}

private:
	long long delay_;
};

/**
 * �첽�ͻ������Ļص��������
 */
class io_callback : public acl::aio_callback {
public:
	explicit io_callback(acl::aio_socket_stream* client)
	: client_(client), i_(0) {}

protected:
	~io_callback() override {
		std::cout << "delete io_callback now ..." << std::endl;
	}

	/**
	 * ʵ�ָ����е��麯�����ͻ������Ķ��ɹ��ص�����
	 * @param data {char*} ���������ݵ�ַ
	 * @param len {int} ���������ݳ���
	 * @return {bool} ���� true ��ʾ����������ϣ���رո��첽��
	 */
	bool read_callback(char* data, int len) override {
		i_++;
		if (i_ < 5) {
			std::cout << ">>gets(i:" << i_ << "): "
				<< data << std::endl;
		}

		// ���Զ�̿ͻ���ϣ���˳�����ر�֮
		if (strncasecmp(data, "quit", 4) == 0) {
			client_->format("Bye!\r\n");
			client_->close();
			return false;
		}

		// ���Զ�̿ͻ���ϣ�������Ҳ�رգ�����ֹ�첽�¼�����
		else if (strncasecmp(data, "stop", 4) == 0) {
			client_->format("Stop now!\r\n");
			client_->close();  // �ر�Զ���첽��

			// ֪ͨ�첽����ر�ѭ������
			client_->get_handle().stop();
		}

		// ��Զ�̿ͻ��˻�д�յ�������

		if (!strncasecmp(data, "write_delay", strlen("write_delay"))) {
			// �ӳ�д����

			const char* ptr = data + strlen("write_delay");
			char *end;
			auto delay = std::strtoll(ptr, &end, 10);
			if (delay > 0) {
				std::cout << ">> write delay " << delay
					<< " second ..." << std::endl;
				auto* timer = new timer_writer(delay);
				client_->write(data, len, delay * 1000000, timer);
				client_->gets(10, false);
				return true;
			}
		} else if (!strncasecmp(data, "read_delay", strlen("read_delay"))) {
			// �ӳٶ�����

			const char* ptr = data + strlen("read_delay");
			char *end;
			auto delay = std::strtoll(ptr, &end, 10);
			if (delay > 0) {
				client_->write(data, len);
				std::cout << ">> read delay " << delay
					<< " second ..." << std::endl;
				auto* timer = new timer_reader(delay);
				client_->gets(10, false, delay * 1000000, timer);
				return true;
			}
		}

		client_->write(data, len);
		//client_->gets(10, false);
		return true;
	}

	/**
	 * ʵ�ָ����е��麯�����ͻ�������д�ɹ��ص�����
	 * @return {bool} ���� true ��ʾ����������ϣ���رո��첽��
	 */
	bool write_callback() override {
		return true;
	}

	/**
	 * ʵ�ָ����е��麯�����ͻ������ĳ�ʱ�ص�����
	 */
	void close_callback() override {
		// �����ڴ˴�ɾ���ö�̬����Ļص�������Է�ֹ�ڴ�й¶
		delete this;
	}

	/**
	 * ʵ�ָ����е��麯�����ͻ������ĳ�ʱ�ص�����
	 * @return {bool} ���� true ��ʾ����������ϣ���رո��첽��
	 */
	bool timeout_callback() override {
		std::cout << "Timeout, delete it ..." << std::endl;
		return false;
	}

private:
	acl::aio_socket_stream* client_;
	int  i_;
};

/**
 * �첽�������Ļص��������
 */
class io_accept_callback : public acl::aio_accept_callback
			 , public acl::aio_listen_callback
{
public:
	io_accept_callback() = default;

	~io_accept_callback() override {
		printf(">>io_accept_callback over!\n");
	}

	/**
	 * ���� aio_accept_callback �麯�������������ӵ������ô˻ص�����
	 * @param client {aio_socket_stream*} �첽�ͻ�����
	 * @return {bool} ���� true ��֪ͨ��������������
	 */
	bool accept_callback(acl::aio_socket_stream* client) override {
		printf("proactor accept one\r\n");
		return handle_client(client);
	}

	/**
	 * ���� aio_listen_callback �麯�������������ӵ������ô˻ص�����
	 * @param server {acl::aio_listen_stream&} �첽������
	 * @return {bool}
	 */
	bool listen_callback(acl::aio_listen_stream& server) override {
		// reactor ģʽ����Ҫ�û��Լ����� accept ����
		acl::aio_socket_stream* client = server.accept();
		if (client == nullptr) {
			printf("accept error %s\r\n", acl::last_serror());
			return false;
		}

		printf("reactor accept one\r\n");
		return handle_client(client);
	}

private:
	static bool handle_client(acl::aio_socket_stream* client) {
		// �����첽�ͻ������Ļص���������첽�����а�
		auto* callback = new io_callback(client);

		// ע���첽���Ķ��ص�����
		client->add_read_callback(callback);

		// ע���첽����д�ص�����
		client->add_write_callback(callback);

		// ע���첽���Ĺرջص�����
		client->add_close_callback(callback);

		// ע���첽���ĳ�ʱ�ص�����
		client->add_timeout_callback(callback);

		// ���޶�����������󳤶�ʱ
		if (s_max > 0) {
			client->set_buf_max(s_max);
		}

		//client->gets(s_timeout, false); // ���첽����һ������
		client->read(0, s_timeout); // ���첽��������
		return true;
	}
};

class mytimer : public acl::aio_timer_callback {
public:
	explicit mytimer(long long delay) : id_(0), last_(time(nullptr)), delay_(delay) {}
	~mytimer() override = default;

protected:
	// @override
	void destroy() override {
		delete this;
	}

	// @override
	void timer_callback(unsigned int id) override {
		time_t now = time(nullptr);
		delay_ += 1000000;
		this->set_task(id_, delay_);

		printf("timer id=%u, delay=%ld, next delay=%lld\r\n",
			id, (long) (now - last_), delay_);

		last_ = now;
		id_ = id;
	}

private:
	unsigned int id_;
	time_t last_;
	long long delay_;
};

//#include <sys/epoll.h>

static void aio_run(bool use_reactor, acl::aio_handle& handle,
	acl::aio_listen_stream* sstream) {
	// �����ص�����󣬵��������ӵ���ʱ�Զ����ô������Ļص�����
	io_accept_callback callback;

#if 0
	struct epoll_event event;
	int pfd = epoll_create(100);
	int epfd = fcntl(pfd, F_DUPFD_CLOEXEC, 0);
	printf(">>>epfd=%d, pfd=%d\n", epfd, pfd);
	event.events = EPOLLIN;
	event.data.ptr = sstream;
	int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, sstream->sock_handle(), &event);
	printf(">>>epoll add ret=%d\n", ret);
	ret = epoll_ctl(epfd, EPOLL_CTL_DEL, sstream->sock_handle(), &event);
	printf(">>>epoll del ret=%d\n", ret);
#endif

	if (use_reactor) {
		sstream->add_listen_callback(&callback);
	} else {
		sstream->add_accept_callback(&callback);
	}

	printf("aio begin running ...\r\n");

	while (true) {
		// ������� false ���ʾ���ټ�������Ҫ�˳�
		if (!handle.check()) {
			std::cout << "aio_server stop now ..." << std::endl;
			break;
		}
	}

	// �رռ��������ͷ�������
	sstream->close();

	// XXX: Ϊ�˱�֤�ܹرռ�������Ӧ�ڴ˴��� check һ��
	handle.check();
}

static acl::aio_listen_stream* bind_addr(acl::aio_handle& handle,
	const acl::string& addr) {
	// ���������첽��
	auto* sstream = new acl::aio_listen_stream(&handle);

	// ����ָ���ĵ�ַ
	if (!sstream->open(addr.c_str())) {
		std::cout << "open " << addr.c_str() << " error!" << std::endl;
		sstream->close();
		// XXX: Ϊ�˱�֤�ܹرռ�������Ӧ�ڴ˴��� check һ��
		handle.check();

		getchar();
		return nullptr;
	}

	std::cout << "Listen: " << addr.c_str() << " ok!" << std::endl;
	return sstream;
}

static void aio_start(const char *addr) {
	if (acl::fiber::scheduled()) {
		printf(">>>RUNNING IN FIBER MODE<<<\r\n");
	} else {
		printf(">>>RUNNING NOT IN FIBER MODE<<<\r\n");
	}

	// �����첽���������
	acl::aio_handle handle(s_use_kernel ?
	       acl::ENGINE_KERNEL : acl::ENGINE_SELECT);

	long long delay = 1000000;
	auto* timer = new mytimer(delay);
	timer->keep_timer(true);
	handle.set_timer(timer, delay);

	auto* sstream = bind_addr(handle, addr);
	if (sstream) {
		aio_run(s_use_reactor, std::ref(handle), sstream);
		sstream->destroy();
	}
}

static void usage(const char* procname) {
	printf("usage: %s -h[help]\r\n"
		" -s ip:port, default: 127.0.0.1:9001\r\n"
		" -d line_max_length\r\n"
		" -t timeout\r\n"
		" -R [use reactor mode other proactor mode, default: proactor mode]\r\n"
		" -F [if use fiber mode]\r\n"
		" -K [use kernel event: epoll/iocp/kqueue/devpool]\r\n",
		procname);
}

int main(int argc, char* argv[]) {
	bool use_fiber = false;
	acl::string addr("127.0.0.1:9001");
	char *dummy;
	int  ch;

	while ((ch = getopt(argc, argv, "s:hKd:t:RF")) > 0) {
		switch (ch) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 's':
			addr = optarg;
			break;
		case 'K':
			s_use_kernel = true;
			break;
		case 'd':
			s_max = (int) std::strtol(optarg, &dummy, 10);
			break;
		case 't':
			s_timeout = (int) std::strtol(optarg, &dummy, 10);
			break;
		case 'R':
			s_use_reactor = true;
			break;
		case 'F':
			use_fiber = true;
			break;
		default:
			break;
		}
	}

	// ��ʼ��ACL��(��������WIN32��һ��Ҫ���ô˺�������UNIXƽ̨�¿ɲ�����)
	acl::acl_cpp_init();

	acl::fiber::stdout_open(true);
	acl::log::stdout_open(true);

	if (use_fiber) {
		go[&] {
			aio_start(addr);
		};

		go[=] {
			while (true) {
				sleep(2);
				printf("---fiber-%d wakeup---\r\n", acl::fiber::self());
			}
		};

		acl::fiber::schedule();
	} else {
		aio_start(addr);
	}

	return 0;
}
