#pragma once

#include <BAN/UniqPtr.h>
#include <BAN/Vector.h>
#include <kernel/Memory/DMARegion.h>
#include <kernel/Timer/RTC.h>

#include <time.h>

namespace Kernel
{

	class Timer
	{
	public:
		virtual ~Timer() {};
		virtual uint64_t ms_since_boot() const = 0;
		virtual uint64_t ns_since_boot() const = 0;
		virtual timespec time_since_boot() const = 0;

		virtual bool pre_scheduler_sleep_needs_lock() const = 0;
		virtual void pre_scheduler_sleep_ns(uint64_t) = 0;

	protected:
		bool should_invoke_scheduler() const { return m_should_invoke_scheduler; }

	private:
		bool m_should_invoke_scheduler { true };
		friend class SystemTimer;
	};

	class SystemTimer : public Timer
	{
	public:
		struct TSCInfo
		{
			int8_t shift;
			uint32_t mult;
		};

	public:
		static void initialize();
		static SystemTimer& get();
		static bool is_initialized();

		void initialize_tsc();

		virtual uint64_t ms_since_boot() const override;
		virtual uint64_t ns_since_boot() const override;
		virtual timespec time_since_boot() const override;

		virtual bool pre_scheduler_sleep_needs_lock() const override;
		virtual void pre_scheduler_sleep_ns(uint64_t) override;

		void sleep_ms(uint64_t ms) const { ASSERT(!BAN::Math::will_multiplication_overflow<uint64_t>(ms, 1'000'000)); return sleep_ns(ms * 1'000'000); }
		void sleep_ns(uint64_t ns) const;

		void dont_invoke_scheduler() { m_timer->m_should_invoke_scheduler = false; }

		void update_tsc();
		TSCInfo tsc_info() const;

		uint64_t ns_since_boot_no_tsc() const { return m_timer->ns_since_boot(); }

		timespec real_time() const;

	private:
		SystemTimer() = default;

		void initialize_timers();

		void initialize_invariant_tsc();
		void initialize_pvclock();

	private:
		enum class TSCType
		{
			None,
			Invariant,
			PVClock,
		};

		uint64_t m_boot_time { 0 };
		BAN::UniqPtr<RTC> m_rtc;
		BAN::UniqPtr<Timer> m_timer;

		TSCType m_tsc_type = TSCType::None;

		union {
			struct {
				int8_t shift;
				uint32_t mult;
			} invariant;
		} m_tsc_info;

		BAN::UniqPtr<DMARegion> m_tsc_page;
		uint64_t m_tsc_update_ns { 0 };
	};

}
