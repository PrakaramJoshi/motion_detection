#ifndef BUFFER_H
#define BUFFER_H
#include <mutex>
#include <queue>
#include <atomic>
#include <thread>
#include <numeric>
#include <condition_variable>
template<class QueueData>class BlockingQueue
{
private:
	std::queue<QueueData*> m_queue;						// Use STL queue to store data
	std::mutex m_mutex;						// The mutex to synchronise on
	std::condition_variable m_cond;		// The condition to wait for
	std::atomic<bool> m_done;
	std::atomic<std::size_t> m_size;
	std::vector<std::size_t> m_q_size;
	int m_maxQSize;

	BlockingQueue(const BlockingQueue &);

	BlockingQueue* operator=(const BlockingQueue &);

	BlockingQueue(BlockingQueue &&);

	BlockingQueue* operator=(BlockingQueue &&);

public:

	BlockingQueue(int _maxQSize = -1)
	{
		m_done = false;
		m_maxQSize = _maxQSize;//_maxQSize;
		m_size = 0;
	}

	~BlockingQueue() {
		ShutDown();
		CleanUp();
	};

	void set_max_size(int _maxQSize) {
		// Acquire lock on the queue
		std::unique_lock<std::mutex> lock(m_mutex);
		m_maxQSize = _maxQSize;
	};

	// Add data to the queue and notify others
	std::size_t Insert(QueueData *_data)
	{
		if (m_done)
			return m_size;

		// Acquire lock on the queue
		std::unique_lock<std::mutex> lock(m_mutex);
		while (m_maxQSize != -1 && m_queue.size() >= m_maxQSize)
			m_cond.wait(lock);
		m_queue.push(_data);
		m_q_size.push_back(m_size + 1);
		lock.unlock();
		// Notify others that data is ready
		m_cond.notify_one();
		m_size++;

		return m_size;
	} // Lock is automatically released here

	  // Get data from the queue. Wait for data if not available
	int Remove(QueueData **_data)
	{
		// Acquire lock on the queue
		std::unique_lock<std::mutex> lock(m_mutex);

		// When there is no data, wait till someone fills it.
		// Lock is automatically released in the wait and obtained
		// again after the wait
		while (m_queue.size() == 0 && !m_done) m_cond.wait(lock);

		if (m_queue.size() == 0 && m_done)
		{
			lock.unlock();
			m_cond.notify_one();
			return 0;
		}
		// Retrieve the data from the queue
		*_data = &(*m_queue.front()); m_queue.pop();
		m_size--;
		lock.unlock();
		m_cond.notify_one();
		return 1;
	}; // Lock is automatically released here;

	   // Get data from the queue. Wait for data if not available
	int Remove_try(QueueData **_data)
	{
		// Acquire lock on the queue
		;
		std::unique_lock<std::mutex> lock(m_mutex);


		if (m_queue.size() == 0 && m_done)
		{
			lock.unlock();
			m_cond.notify_one();
			return 0;
		}
		if (m_queue.size() == 0) {
			*_data = nullptr;
			lock.unlock();
			m_cond.notify_one();
			return 1;
		}
		// Retrieve the data from the queue
		*_data = &(*m_queue.front()); m_queue.pop();
		m_size--;
		lock.unlock();
		m_cond.notify_one();
		return 1;
	}; // Lock is automatically released here;

	   // Get data from the queue. Wait for data if not available
	int CanInsert()
	{
		if (m_done)
			return 0;
		return 1;
	}; // Lock is automatically released here;

	std::size_t size() {
		return m_size;
	}

	void ShutDown()
	{
		m_done = true;
		m_cond.notify_one();
	}

	void Restart() {
		CleanUp();
		m_done = false;
		m_cond.notify_one();
	}

	bool IsShutDown() {
		return m_done.load();
	}

	double AverageQSize() {
		double average = 0;
		if (m_q_size.size() > 1) {
			double total = std::accumulate(m_q_size.begin(), m_q_size.end(), 0);
			average = total / m_q_size.size();
		}
		return average;

	}

	void CleanUp()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		while (!m_queue.empty())
		{
			m_queue.pop();
			m_size--;
		}
	}
};

#endif