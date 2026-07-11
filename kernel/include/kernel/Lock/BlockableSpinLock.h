#pragma once

#include <kernel/Lock/SpinLock.h>
#include <kernel/Lock/Mutex.h>

namespace Kernel
{

	// FIXME: These classes are HACKS to allow passing spinlock
	//        to unblock functions. Write a better API that either
	//        allows passing spinlocks or do something cleaner that
	//        whatever shit this is

	template<typename Lock> requires requires (Lock& lock) { lock.lock(); lock.unlock(InterruptState::Disabled); lock.current_processor_has_lock(); }
	class BlockableSpinLock : public BaseMutex
	{
	public:
		BlockableSpinLock(Lock& lock)
			: m_lock(lock)
		{
			ASSERT(m_lock.current_processor_has_lock());
		}

		void lock() override
		{
			m_lock.lock();
		}

		void unlock() override
		{
			m_lock.unlock(InterruptState::Disabled);
		}

		uint32_t lock_depth() const override { return m_lock.lock_depth(); }
		bool is_locked_by_current_thread() const override { return m_lock.current_processor_has_lock(); }

	private:
		Lock& m_lock;
	};

}
