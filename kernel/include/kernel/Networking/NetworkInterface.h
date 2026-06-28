#pragma once

#include <BAN/ByteSpan.h>
#include <BAN/Errors.h>
#include <BAN/IPv4.h>
#include <BAN/MAC.h>
#include <kernel/Device/Device.h>

namespace Kernel
{

	struct EthernetHeader
	{
		BAN::MACAddress dst_mac;
		BAN::MACAddress src_mac;
		BAN::NetworkEndian<uint16_t> ether_type;
	};
	static_assert(sizeof(EthernetHeader) == 14);

	enum EtherType : uint16_t
	{
		IPv4 = 0x0800,
		ARP = 0x0806,
	};

	enum NetworkChecksum : uint32_t
	{
		CKSUM_IPV4 = 1 << 0,
		CKSUM_TCP  = 1 << 1,
		CKSUM_UDP  = 1 << 2,
	};

	class NetworkInterface : public CharacterDevice
	{
		BAN_NON_COPYABLE(NetworkInterface);
		BAN_NON_MOVABLE(NetworkInterface);

	public:
		enum class Type
		{
			Ethernet,
			Loopback,
		};

	public:
		NetworkInterface(Type);
		virtual ~NetworkInterface() {}

		virtual BAN::MACAddress get_mac_address() const = 0;

		BAN::IPv4Address get_ipv4_address() const { return m_ipv4_address; }
		void set_ipv4_address(BAN::IPv4Address new_address) { m_ipv4_address = new_address; }

		BAN::IPv4Address get_netmask() const { return m_netmask; }
		void set_netmask(BAN::IPv4Address new_netmask) { m_netmask = new_netmask; }

		BAN::IPv4Address get_gateway() const { return m_gateway; }
		void set_gateway(BAN::IPv4Address new_gateway) { m_gateway = new_gateway; }

		Type type() const { return m_type; }

		virtual bool link_up() = 0;
		virtual int link_speed() = 0;

		virtual size_t payload_mtu() const = 0;

		virtual BAN::StringView name() const override { return m_name; }

		BAN::ErrorOr<void> send_with_ethernet_header(BAN::MACAddress dst_mac, EtherType ether_type, BAN::ConstByteSpan buffer)
		{
			BAN::ConstByteSpan buffer_array[1] { buffer };
			return send_with_ethernet_header(dst_mac, ether_type, buffer_array);
		}

		template<size_t SIZE>
		BAN::ErrorOr<void> send_with_ethernet_header(BAN::MACAddress dst_mac, EtherType ether_type, const BAN::ConstByteSpan (&buffer_array)[SIZE])
		{
			const auto ethernet_header = EthernetHeader {
				.dst_mac = dst_mac,
				.src_mac = get_mac_address(),
				.ether_type = ether_type,
			};

			BAN::ConstByteSpan new_buffer_array[SIZE + 1];
			new_buffer_array[0] = BAN::ConstByteSpan::from(ethernet_header);
			for (size_t i = 0; i < SIZE; i++)
				new_buffer_array[i + 1] = buffer_array[i];

			return send_raw_bytes({ new_buffer_array, SIZE + 1 });
		}

		virtual BAN::ErrorOr<void> send_raw_bytes(BAN::Span<const BAN::ConstByteSpan> buffers) = 0;

	private:
		BAN::ErrorOr<long> ioctl_impl(int, void*) override;

	private:
		const Type m_type;
		char m_name[10];

		BAN::IPv4Address m_ipv4_address { 0 };
		BAN::IPv4Address m_netmask { 0 };
		BAN::IPv4Address m_gateway { 0 };
	};

}
