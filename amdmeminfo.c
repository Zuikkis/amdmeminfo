/*
 * AMDMemInfo, (c) 2014 by Zuikkis <zuikkis@gmail.com>
 *
 * Loosely based on "amdmeminfo" by Joerie de Gram.
 *
 * AMDMemInfo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AMDMemInfo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AMDMemInfo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pci/pci.h>
#include <stdbool.h>

/*
 * Find all suitable cards, then find their memory space and get memory information.
 */
int main()
{
	struct pci_access *pci;
	struct pci_dev *dev;
	int i, meminfo;
	u32 subvendor, subdevice;
	off_t base;
	int *pcimem;
	int fd;
	char *devname;
	int fail =0, manufacturer, model;

	printf("AMDMemInfo by Zuikkis <zuikkis@gmail.com>\n");

	pci = pci_alloc();
	pci_init(pci);

	pci_scan_bus(pci);	


	for(dev = pci->devices; dev; dev = dev->next) {
	    if(dev->device_class == PCI_CLASS_DISPLAY_VGA &&
	      dev->vendor_id == 0x1002 && 
	    ( dev->device_id == 0x679a ||   // hd7950
	      dev->device_id == 0x6798 ||   // hd7970 / r9 280x
	      dev->device_id == 0x67b1 ||   // r9 290
	      dev->device_id == 0x67b0 ))    // r9 290
 
	    {
			switch (dev->device_id) {
				case 0x679a: devname="Radeon HD7950"; 			break;
				case 0x6798: devname="Radeon HD7970 / R9 280x"; break;
				case 0x67b1: devname="Radeon R9 290"; 			break;
				case 0x67b0: devname="Radeon R9 290x"; 			break;
				default: devname="Unknown"; break;
			}

			subvendor = pci_read_word(dev, PCI_SUBSYSTEM_VENDOR_ID);
			subdevice = pci_read_word(dev, PCI_SUBSYSTEM_ID);

			printf(	"-----------------------------------\n"
					"Found card:  %04x:%04x (AMD %s)\n"
					"Subvendor:   0x%x\n"
					"Subdevice:   0x%x\n",
					dev->vendor_id, dev->device_id, devname, subvendor, subdevice);

			for (i=0;i<6;i++) {
			    if (dev->size[i]==0x40000) {
				base=(dev->base_addr[i] & 0xfffffff0);

     			  	fd = open ( "/dev/mem", O_RDWR);
  			  	pcimem = (int *) mmap(NULL, 0x20000, PROT_READ, MAP_SHARED, fd, base);
				if (pcimem == MAP_FAILED) {
				    fail++;
				} else {
				    meminfo=pcimem[(0x2a00)/4];
		 		    manufacturer=(meminfo & 0xf00)>>8;
				    model=(meminfo & 0xf000)>>12;
				    printf("Memory type: ");
				    switch(manufacturer) {
					case 1:
					    printf("Samsung K4G20325FD\n"); break;
					case 3:
					    printf("Elpida EDW2032BBBG\n"); break;
					case 6:
					    switch(model) {
						case 2: printf("SK Hynix H5GQ2H24MFR\n"); break;
						case 3: printf("SK Hynix H5GQ2H24AFR\n"); break;
						default: printf("SK Hynix unknown model %d\n",model);
					    }
					    break;
					default:
					    printf("Unknown manufacturer %d\n",manufacturer);
				    }

	 	 		    munmap(pcimem, 0x20000);
				}
				close(fd);
				}
			}
		}
	}

	pci_cleanup(pci);

	if (fail) {
		printf("Direct PCI access failed. Run AMDMemInfo as root to get memory type information!\n");
	}
	return 0;
}

