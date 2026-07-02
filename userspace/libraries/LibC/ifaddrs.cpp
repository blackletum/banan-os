#include <dirent.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

static int selector(const dirent* dirent)
{
	if (strcmp(dirent->d_name, "lo") == 0)
		return 1;
	if (strncmp(dirent->d_name, "eth", 3) == 0)
		return 1;
	return 0;
}

static int comparator(const dirent** d1, const dirent** d2)
{
	if (strcmp((*d1)->d_name, "lo"))
		return -1;
	if (strcmp((*d2)->d_name, "lo"))
		return +1;
	return alphasort(d1, d2);
}

static void free_ifaddrs(ifaddrs* ifa)
{
	if (ifa->ifa_name)
		free(ifa->ifa_name);
	if (ifa->ifa_addr)
		free(ifa->ifa_addr);
	if (ifa->ifa_netmask)
		free(ifa->ifa_netmask);
	if (ifa->ifa_ifu.ifu_broadaddr)
		free(ifa->ifa_ifu.ifu_broadaddr);
	if (ifa->ifa_data)
		free(ifa->ifa_data);
	free(ifa);
}

static sockaddr* dup_sockaddr(const sockaddr* saddr)
{
	size_t size;
	switch (saddr->sa_family)
	{
		case AF_INET:
			size = sizeof(sockaddr_in);
			break;
		case AF_INET6:
			size = sizeof(sockaddr_in6);
			break;
		default:
			return nullptr;
	}

	sockaddr* result = static_cast<sockaddr*>(malloc(size));
	if (result == nullptr)
		return nullptr;
	memcpy(result, saddr, size);
	return result;
}

static ifaddrs* open_ifaddrs(const char* path)
{
	int fd = open(path, O_RDONLY);
	if (fd == -1)
		return nullptr;

	ifaddrs* ifa = static_cast<ifaddrs*>(calloc(1, sizeof(ifaddrs)));
	if (ifa == nullptr)
		goto open_ifaddrs_error;

	ifreq ifreq;

	if (ioctl(fd, SIOCGIFNAME, &ifreq) == -1)
		goto open_ifaddrs_error;
	ifa->ifa_name = strdup(ifreq.ifr_name);
	if (ifa->ifa_name == nullptr)
		goto open_ifaddrs_error;

	if (ioctl(fd, SIOCGIFFLAGS, &ifreq) == -1)
		goto open_ifaddrs_error;
	ifa->ifa_flags = ifreq.ifr_flags;

	if (ioctl(fd, SIOCGIFADDR, &ifreq) == -1)
		goto open_ifaddrs_error;
	ifa->ifa_addr = dup_sockaddr(&ifreq.ifr_addr);
	if (ifa->ifa_addr == nullptr)
		goto open_ifaddrs_error;

	if (ioctl(fd, SIOCGIFNETMASK, &ifreq) == -1)
		goto open_ifaddrs_error;
	ifa->ifa_netmask = dup_sockaddr(&ifreq.ifr_netmask);
	if (ifa->ifa_netmask == nullptr)
		goto open_ifaddrs_error;

	// TODO: broadcast/point-to-point

	close(fd);
	return ifa;

open_ifaddrs_error:
	if (ifa != nullptr)
		free_ifaddrs(ifa);
	close(fd);
	return nullptr;
}

int getifaddrs(struct ifaddrs** ifap)
{
	dirent** namelist;

	const int count = scandir("/dev", &namelist, selector, comparator);
	if (count == -1)
		return -1;

	*ifap = nullptr;

	for (int i = 0; i < count; i++)
	{
		char path[5 + sizeof(namelist[i]->d_name)];
		sprintf(path, "/dev/%s", namelist[i]->d_name);

		ifaddrs* ifa = open_ifaddrs(path);
		if (ifa == nullptr)
			continue;
		*ifap = ifa;
		ifap = &ifa->ifa_next;
	}

	for (int i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);

	return 0;
}

void freeifaddrs(struct ifaddrs* ifa)
{
	while (ifa)
	{
		auto* next = ifa->ifa_next;
		free_ifaddrs(ifa);
		ifa = next;
	}
}
