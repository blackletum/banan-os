#pragma once

#include <BAN/UniqPtr.h>
#include <kernel/PCI.h>
#include <kernel/Storage/ATA/ATABus.h>
#include <kernel/Storage/ATA/ATADevice.h>

namespace Kernel
{

	class ATAController : public BAN::RefCounted<ATAController>
	{
	public:
		static BAN::ErrorOr<BAN::RefPtr<ATAController>> create(PCI::Device&);

	private:
		ATAController(PCI::Device& pci_device)
			: m_pci_device(pci_device)
		{ }
		BAN::ErrorOr<void> initialize();

	private:
		PCI::Device& m_pci_device;
	};

}
