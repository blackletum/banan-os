#pragma once

#include <kernel/Memory/DMARegion.h>
#include <kernel/Storage/ATA/AHCI/Definitions.h>
#include <kernel/Storage/ATA/ATADevice.h>
#include <kernel/ThreadBlocker.h>

namespace Kernel
{

	class AHCIDevice final : public detail::ATABaseDevice
	{
	public:
		static BAN::ErrorOr<BAN::RefPtr<AHCIDevice>> create(BAN::RefPtr<AHCIController>, volatile HBAPortMemorySpace*);
		~AHCIDevice() = default;

	private:
		AHCIDevice(BAN::RefPtr<AHCIController> controller, volatile HBAPortMemorySpace* port)
			: m_controller(controller)
			, m_port(port)
		{ }
		BAN::ErrorOr<void> initialize();
		BAN::ErrorOr<void> rebase();
		BAN::ErrorOr<void> read_identify_data();

		paddr_t read_paddr(volatile uint32_t& lo, volatile uint32_t& hi) const;
		void write_paddr(volatile uint32_t& lo, volatile uint32_t& hi, paddr_t paddr);

		bool can_use_buffer_directly(BAN::ConstByteSpan buffer) const;

		virtual BAN::ErrorOr<void> read_sectors_impl(uint64_t lba, uint64_t sector_count, BAN::ByteSpan) override;
		virtual BAN::ErrorOr<void> write_sectors_impl(uint64_t lba, uint64_t sector_count, BAN::ConstByteSpan) override;
		BAN::ErrorOr<size_t> send_command_sync(uint64_t lba, BAN::ConstByteSpan buffer, Command command);

		BAN::ErrorOr<void> send_command_and_wait(uint32_t slot);
		BAN::ErrorOr<uint32_t> find_free_command_slot();

		void handle_irq();

	private:
		static constexpr uint32_t m_max_hba_prdt_count { 64 };

		BAN::Atomic<uint32_t> m_prev_is { 0 };
		uint32_t m_free_slots { 0 };

		BAN::RefPtr<AHCIController> m_controller;
		volatile HBAPortMemorySpace* const m_port;

		BAN::UniqPtr<DMARegion> m_dma_region;

		Mutex m_temp_buffer_mutex;
		BAN::UniqPtr<DMARegion> m_temp_buffer;

		SpinLock m_command_lock;
		ThreadBlocker m_command_blocker;

		friend class AHCIController;
	};

}
