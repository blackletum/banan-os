#pragma once

#include <BAN/Array.h>
#include <BAN/RefPtr.h>
#include <kernel/Interruptable.h>
#include <kernel/Memory/DMARegion.h>
#include <kernel/PCI.h>
#include <kernel/Storage/ATA/AHCI/Definitions.h>

namespace Kernel
{

	class AHCIController final : public BAN::RefCounted<AHCIController>, public Interruptable
	{
		BAN_NON_COPYABLE(AHCIController);
		BAN_NON_MOVABLE(AHCIController);

	public:
		static BAN::ErrorOr<BAN::RefPtr<AHCIController>> create(PCI::Device&);

		~AHCIController();

		virtual void handle_irq() override;

		bool supports_64bit() const { return m_supports_64bit; }
		uint32_t command_slot_count() const { return m_command_slot_count; }

	private:
		AHCIController(PCI::Device& pci_device)
			: m_pci_device(pci_device)
		{ }
		BAN::ErrorOr<void> initialize();

		BAN::Optional<AHCIPortType> check_port_type(volatile HBAPortMemorySpace&);

	private:
		PCI::Device& m_pci_device;
		BAN::UniqPtr<PCI::BarRegion> m_abar;

		BAN::Array<AHCIDevice*, 32> m_devices;

		bool m_supports_64bit { false };
		uint32_t m_command_slot_count { 0 };
	};

}
