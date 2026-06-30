#include <BAN/Sort.h>

#include <kernel/CPUID.h>
#include <kernel/Scheduler.h>
#include <kernel/Timer/HPET.h>
#include <kernel/Timer/PIT.h>
#include <kernel/Timer/Timer.h>

namespace Kernel
{

	static SystemTimer* s_instance = nullptr;

	struct pvclock_vcpu_time_info
	{
		uint32_t version;
		uint32_t pad0;
		uint64_t tsc_timestamp;
		uint64_t system_time;
		uint32_t tsc_to_system_mul;
		int8_t tsc_shift;
		uint8_t flags;
		uint8_t pad[2];
	};

	void SystemTimer::initialize()
	{
		ASSERT(s_instance == nullptr);
		auto* temp = new SystemTimer;
		ASSERT(temp);
		temp->initialize_timers();
		s_instance = temp;
	}

	SystemTimer& SystemTimer::get()
	{
		ASSERT(s_instance);
		return *s_instance;
	}

	bool SystemTimer::is_initialized()
	{
		return !!s_instance;
	}

	void SystemTimer::initialize_timers()
	{
		m_rtc = MUST(BAN::UniqPtr<RTC>::create());
		m_boot_time = BAN::to_unix_time(m_rtc->get_current_time());

		if (auto res = HPET::create(); res.is_error())
			dwarnln("HPET: {}", res.error());
		else
		{
			m_timer = res.release_value();
			dprintln("HPET initialized");
			return;
		}

		if (auto res = PIT::create(); res.is_error())
			dwarnln("PIT: {}", res.error());
		else
		{
			m_timer = res.release_value();
			dprintln("PIT initialized");
			return;
		}

		Kernel::panic("Could not initialize any timer");
	}

	void SystemTimer::initialize_tsc()
	{
		if (CPUID::has_kvm_pvclock())
			return initialize_pvclock();
		if (CPUID::has_invariant_tsc())
			return initialize_invariant_tsc();
		dwarnln("No supported TSC based timers available");
	}

	void SystemTimer::initialize_invariant_tsc()
	{
		const uint64_t tsc_freq = [this]() -> uint64_t {
			if (const auto cpuid_freq = CPUID::get_tsc_frequency())
				return cpuid_freq;

			// take 5x 50 ms samples and use the median value

			constexpr size_t tsc_sample_count = 5;
			constexpr size_t tsc_sample_ns = 50'000'000;

			uint64_t tsc_freq_samples[tsc_sample_count];
			for (size_t i = 0; i < tsc_sample_count; i++)
			{
				const auto start_ns = m_timer->ns_since_boot();

				const auto start_tsc = __builtin_ia32_rdtsc();
				while (m_timer->ns_since_boot() < start_ns + tsc_sample_ns)
					Processor::pause();
				const auto stop_tsc = __builtin_ia32_rdtsc();

				const auto stop_ns = m_timer->ns_since_boot();

				const auto duration_ns = stop_ns - start_ns;
				const auto count_tsc = stop_tsc - start_tsc;

				tsc_freq_samples[i] = count_tsc * 1'000'000'000 / duration_ns;
			}

			BAN::sort::sort(tsc_freq_samples, tsc_freq_samples + tsc_sample_count);

			return tsc_freq_samples[tsc_sample_count / 2];
		}();

		m_tsc_info = { .invariant = {
			.shift = 0,
			.mult  = static_cast<uint32_t>((1'000'000'000ull << 32) / tsc_freq),
		}};
		m_tsc_type = TSCType::Invariant;
		Processor::initialize_tsc(m_boot_time);

		dprintln("Initialized invariant TSC ({} Hz)", tsc_freq);
	}

	static pvclock_vcpu_time_info read_pvclock_safe(vaddr_t pvclock_vaddr)
	{
		for (;;)
		{
			const volatile auto& pvclock = *reinterpret_cast<const volatile pvclock_vcpu_time_info*>(pvclock_vaddr);

			const auto version = pvclock.version;
			if (version & 1)
				continue;

			pvclock_vcpu_time_info copy;
			memcpy(&copy, const_cast<const pvclock_vcpu_time_info*>(&pvclock), sizeof(pvclock_vcpu_time_info));

			if (pvclock.version == version)
				return copy;
		}
	}

	void SystemTimer::initialize_pvclock()
	{
		m_tsc_page = MUST(DMARegion::create(sizeof(pvclock_vcpu_time_info), PageTable::MemoryType::Normal));
		memset(reinterpret_cast<void*>(m_tsc_page->vaddr()), 0, sizeof(pvclock_vcpu_time_info));

		const uint32_t paddr_hi = m_tsc_page->paddr() >> 32;
		const uint32_t paddr_lo = m_tsc_page->paddr() & 0xFFFFFFFF;
		asm volatile("wrmsr" :: "d"(paddr_hi), "a"(paddr_lo | 1), "c"(0x4b564d01));

		m_tsc_type = TSCType::PVClock;
		Processor::initialize_tsc(m_boot_time);

		dprintln("Initialized pvclock");
	}

	void SystemTimer::update_tsc()
	{
		if (m_tsc_type == TSCType::None)
			return;

		// only update once per second
		const uint64_t current_ns = Processor::ns_since_boot_tsc();
		if (current_ns < m_tsc_update_ns)
			return;
		m_tsc_update_ns = current_ns + 1'000'000'000;

		Processor::update_tsc();
		Processor::broadcast_smp_message({
			.type = Processor::SMPMessage::Type::UpdateTSC,
			.dummy = 0,
		});
	}

	SystemTimer::TSCInfo SystemTimer::tsc_info() const
	{
		switch (m_tsc_type)
		{
			case TSCType::None:
				ASSERT_NOT_REACHED();
			case TSCType::Invariant:
				return {
					.shift = m_tsc_info.invariant.shift,
					.mult  = m_tsc_info.invariant.mult,
				};
			case TSCType::PVClock:
				const auto pvclock = read_pvclock_safe(m_tsc_page->vaddr());
				return {
					.shift = pvclock.tsc_shift,
					.mult  = pvclock.tsc_to_system_mul,
				};
		}
		ASSERT_NOT_REACHED();
	}

	uint64_t SystemTimer::ms_since_boot() const
	{
		if (m_tsc_type == TSCType::None)
			return m_timer->ms_since_boot();
		return Processor::ns_since_boot_tsc() / 1'000'000;
	}

	uint64_t SystemTimer::ns_since_boot() const
	{
		if (m_tsc_type == TSCType::None)
			return m_timer->ns_since_boot();
		return Processor::ns_since_boot_tsc();
	}

	timespec SystemTimer::time_since_boot() const
	{
		if (m_tsc_type == TSCType::None)
			return m_timer->time_since_boot();
		const auto ns_since_boot = Processor::ns_since_boot_tsc();
		return {
			.tv_sec = static_cast<time_t>(ns_since_boot / 1'000'000'000),
			.tv_nsec = static_cast<long>(ns_since_boot % 1'000'000'000)
		};
	}

	bool SystemTimer::pre_scheduler_sleep_needs_lock() const
	{
		return m_timer->pre_scheduler_sleep_needs_lock();
	}

	void SystemTimer::pre_scheduler_sleep_ns(uint64_t ns)
	{
		return m_timer->pre_scheduler_sleep_ns(ns);
	}

	void SystemTimer::sleep_for_ns(uint64_t timeout_ns) const
	{
		const uint64_t base_ns = ns_since_boot();
		ASSERT(!BAN::Math::will_addition_overflow(base_ns, timeout_ns));
		Processor::scheduler().block_current_thread(nullptr, base_ns + timeout_ns, nullptr);
	}

	void SystemTimer::sleep_until_ns(uint64_t waketime_ns) const
	{
		if (ns_since_boot() >= waketime_ns)
			return;
		Processor::scheduler().block_current_thread(nullptr, waketime_ns, nullptr);
	}

	timespec SystemTimer::real_time() const
	{
		auto result = time_since_boot();
		result.tv_sec += m_boot_time;
		return result;
	}

}
