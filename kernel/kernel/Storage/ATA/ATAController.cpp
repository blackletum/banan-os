#include <kernel/FS/DevFS/FileSystem.h>
#include <kernel/InterruptController.h>
#include <kernel/Storage/ATA/AHCI/Controller.h>
#include <kernel/Storage/ATA/ATABus.h>
#include <kernel/Storage/ATA/ATAController.h>
#include <kernel/Storage/ATA/ATADefinitions.h>
#include <kernel/Storage/ATA/ATADevice.h>

namespace Kernel
{

	BAN::ErrorOr<BAN::RefPtr<ATAController>> ATAController::create(PCI::Device& pci_device)
	{
		auto* controller_ptr = new ATAController(pci_device);
		if (controller_ptr == nullptr)
			return BAN::Error::from_errno(ENOMEM);
		auto controller = BAN::RefPtr<ATAController>::adopt(controller_ptr);
		TRY(controller->initialize());
		return controller;
	}

	BAN::ErrorOr<void> ATAController::initialize()
	{
		BAN::Vector<BAN::RefPtr<ATABus>> buses;

		uint8_t prog_if = m_pci_device.read_byte(0x09);

		// FIXME: support native mode

		if ((prog_if & ATA_PROGIF_CAN_MODIFY_PRIMARY_NATIVE) && (prog_if & ATA_PROGIF_PRIMARY_NATIVE))
		{
			prog_if &= ~ATA_PROGIF_PRIMARY_NATIVE;
			m_pci_device.write_byte(0x09, prog_if);
			dprintln("enabling compatibility mode for bus 1");
		}

		if ((prog_if & ATA_PROGIF_CAN_MODIFY_SECONDARY_NATIVE) && (prog_if & ATA_PROGIF_SECONDARY_NATIVE))
		{
			prog_if &= ~ATA_PROGIF_SECONDARY_NATIVE;
			m_pci_device.write_byte(0x09, prog_if);
			dprintln("enabling compatibility mode for bus 2");
		}

		if (!(prog_if & ATA_PROGIF_PRIMARY_NATIVE))
		{
			if (InterruptController::get().reserve_irq(14).is_error())
				dwarnln("Could not reserve interrupt {} for ATA device", 14);
			else
			{
				auto bus_or_error = ATABus::create(0x1F0, 0x3F6, 14);
				if (bus_or_error.is_error())
					dprintln("IDE ATABus: {}", bus_or_error.error());
				else
					TRY(buses.push_back(bus_or_error.release_value()));
			}
		}
		else
		{
			dprintln("unsupported IDE ATABus in native mode");
		}

		if (!(prog_if & ATA_PROGIF_SECONDARY_NATIVE))
		{
			if (InterruptController::get().reserve_irq(15).is_error())
				dwarnln("Could not reserver interrupt {} for ATA device", 15);
			else
			{
				auto bus_or_error = ATABus::create(0x170, 0x376, 15);
				if (bus_or_error.is_error())
					dprintln("IDE ATABus: {}", bus_or_error.error());
				else
					TRY(buses.push_back(bus_or_error.release_value()));
			}
		}
		else
		{
			dprintln("unsupported IDE ATABus in native mode");
		}

		return {};
	}

}
