/*
 *  linux/arch/arm/mm/iomap.c
 *
 * Map IO port and PCI memory spaces so that {read,write}[bwl] can
 * be used to access this memory.
 */
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <asm/pci.h>

#ifdef __io
void __iomem *ioport_map(unsigned long port, unsigned int nr)
{
	return __io(port);
}
EXPORT_SYMBOL(ioport_map);

void ioport_unmap(void __iomem *addr)
{
}
EXPORT_SYMBOL(ioport_unmap);
#endif

#ifdef CONFIG_PCI
unsigned int pci_flags = PCI_REASSIGN_ALL_RSRC;
EXPORT_SYMBOL(pci_flags);

void pci_iounmap(struct pci_dev *dev, void __iomem *addr)
{
	if ((unsigned long)addr >= VMALLOC_START &&
	    (unsigned long)addr < VMALLOC_END)
		iounmap(addr);
}
EXPORT_SYMBOL(pci_iounmap);
#endif
