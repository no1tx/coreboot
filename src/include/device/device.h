#ifndef DEVICE_H

#define DEVICE_H

/*
 * NOTICE: Header is ROMCC tentative.
 * This header is incompatible with ROMCC and its inclusion leads to 'odd'
 * build failures.
 */
#if !defined(__ROMCC__)

#include <stdint.h>
#include <stddef.h>
#include <device/resource.h>
#include <device/path.h>
#include <device/pci_type.h>

struct device;
struct pci_operations;
struct pci_bus_operations;
struct i2c_bus_operations;
struct smbus_bus_operations;
struct pnp_mode_ops;
struct spi_bus_operations;
struct usb_bus_operations;

/* Chip operations */
struct chip_operations {
	void (*enable_dev)(struct device *dev);
	void (*init)(void *chip_info);
	void (*final)(void *chip_info);
	unsigned int initialized : 1;
	unsigned int finalized : 1;
	const char *name;
};

#define CHIP_NAME(X) .name = X,

struct bus;

struct smbios_type11;
struct acpi_rsdp;

struct device_operations {
	void (*read_resources)(struct device *dev);
	void (*set_resources)(struct device *dev);
	void (*enable_resources)(struct device *dev);
	void (*init)(struct device *dev);
	void (*final)(struct device *dev);
	void (*scan_bus)(struct device *bus);
	void (*enable)(struct device *dev);
	void (*disable)(struct device *dev);
	void (*set_link)(struct device *dev, unsigned int link);
	void (*reset_bus)(struct bus *bus);
#if IS_ENABLED(CONFIG_GENERATE_SMBIOS_TABLES)
	int (*get_smbios_data)(struct device *dev, int *handle,
		unsigned long *current);
	void (*get_smbios_strings)(struct device *dev, struct smbios_type11 *t);
#endif
#if IS_ENABLED(CONFIG_HAVE_ACPI_TABLES)
	unsigned long (*write_acpi_tables)(struct device *dev,
		unsigned long start, struct acpi_rsdp *rsdp);
	void (*acpi_fill_ssdt_generator)(struct device *dev);
	void (*acpi_inject_dsdt_generator)(struct device *dev);
	const char *(*acpi_name)(const struct device *dev);
#endif
	const struct pci_operations *ops_pci;
	const struct i2c_bus_operations *ops_i2c_bus;
	const struct spi_bus_operations *ops_spi_bus;
	const struct smbus_bus_operations *ops_smbus_bus;
	const struct pci_bus_operations * (*ops_pci_bus)(struct device *dev);
	const struct pnp_mode_ops *ops_pnp_mode;
};

/**
 * Standard device operations function pointers shims.
 */
static inline void device_noop(struct device *dev) {}
#define DEVICE_NOOP device_noop

struct bus {

	DEVTREE_CONST struct device *dev;	/* This bridge device */
	DEVTREE_CONST struct device *children;	/* devices behind this bridge */
	DEVTREE_CONST struct bus *next;    /* The next bridge on this device */
	unsigned int	bridge_ctrl;	/* Bridge control register */
	uint16_t	bridge_cmd;		/* Bridge command register */
	unsigned char	link_num;	/* The index of this link */
	uint16_t	secondary;	/* secondary bus number */
	uint16_t	subordinate;	/* max subordinate bus number */
	unsigned char   cap;		/* PCi capability offset */
	uint32_t	hcdn_reg;		/* For HyperTransport link  */

	unsigned int	reset_needed : 1;
	unsigned int	disable_relaxed_ordering : 1;
	unsigned int	ht_link_up : 1;
};

/*
 * There is one device structure for each slot-number/function-number
 * combination:
 */

struct pci_irq_info {
	unsigned int	ioapic_irq_pin;
	unsigned int	ioapic_src_pin;
	unsigned int	ioapic_dst_id;
	unsigned int    ioapic_flags;
};

struct device {
	DEVTREE_CONST struct bus *bus;	/* bus this device is on, for bridge
					 * devices, it is the up stream bus */

	DEVTREE_CONST struct device *sibling;	/* next device on this bus */

	DEVTREE_CONST struct device *next;	/* chain of all devices */

	struct device_path path;
	unsigned int	vendor;
	unsigned int	device;
	u16		subsystem_vendor;
	u16		subsystem_device;
	unsigned int	class;		/* 3 bytes: (base, sub, prog-if) */
	unsigned int	hdr_type;	/* PCI header type */
	unsigned int    enabled : 1;	/* set if we should enable the device */
	unsigned int  initialized : 1; /* 1 if we have initialized the device */
	unsigned int    on_mainboard : 1;
	unsigned int    disable_pcie_aspm : 1;
	unsigned int    hidden : 1;	/* set if we should hide from UI */
	struct pci_irq_info pci_irq_info[4];
	u8 command;

	/* Base registers for this device. I/O, MEM and Expansion ROM */
	DEVTREE_CONST struct resource *resource_list;

	/* links are (downstream) buses attached to the device, usually a leaf
	 * device with no children has 0 buses attached and a bridge has 1 bus
	 */
	DEVTREE_CONST struct bus *link_list;

	struct device_operations *ops;
#if !DEVTREE_EARLY
	struct chip_operations *chip_ops;
	const char *name;
#endif
	DEVTREE_CONST void *chip_info;
};

/**
 * This is the root of the device tree. The device tree is defined in the
 * static.c file and is generated by the config tool at compile time.
 */
extern DEVTREE_CONST struct device	dev_root;
/* list of all devices */
extern DEVTREE_CONST struct device * DEVTREE_CONST all_devices;
extern struct resource	*free_resources;
extern struct bus	*free_links;

extern const char mainboard_name[];

#if IS_ENABLED(CONFIG_GFXUMA)
/* IGD UMA memory */
extern uint64_t uma_memory_base;
extern uint64_t uma_memory_size;
#endif

/* Generic device interface functions */
struct device *alloc_dev(struct bus *parent, struct device_path *path);
void dev_initialize_chips(void);
void dev_enumerate(void);
void dev_configure(void);
void dev_enable(void);
void dev_initialize(void);
void dev_optimize(void);
void dev_finalize(void);
void dev_finalize_chips(void);

/* Generic device helper functions */
int reset_bus(struct bus *bus);
void scan_bridges(struct bus *bus);
void assign_resources(struct bus *bus);
const char *dev_name(struct device *dev);
const char *dev_path(const struct device *dev);
u32 dev_path_encode(const struct device *dev);
const char *bus_path(struct bus *bus);
void dev_set_enabled(struct device *dev, int enable);
void disable_children(struct bus *bus);
bool dev_is_active_bridge(struct device *dev);

/* Option ROM helper functions */
void run_bios(struct device *dev, unsigned long addr);

/* Helper functions */
DEVTREE_CONST struct device *find_dev_path(
		const struct bus *parent,
		const struct device_path *path);
struct device *alloc_find_dev(struct bus *parent, struct device_path *path);
struct device *dev_find_device(u16 vendor, u16 device, struct device *from);
struct device *dev_find_class(unsigned int class, struct device *from);
DEVTREE_CONST struct device *dev_find_path(
		DEVTREE_CONST struct device *prev_match,
		enum device_path_type path_type);
struct device *dev_find_lapic(unsigned int apic_id);
int dev_count_cpu(void);

struct device *add_cpu_device(struct bus *cpu_bus, unsigned int apic_id,
				int enabled);
void set_cpu_topology(struct device *cpu, unsigned int node,
	unsigned int package, unsigned int core, unsigned int thread);

#define amd_cpu_topology(cpu, node, core) \
	set_cpu_topology(cpu, node, 0, core, 0)

#define intel_cpu_topology(cpu, package, core, thread) \
	set_cpu_topology(cpu, 0, package, core, thread)

/* Debug functions */
void print_resource_tree(const struct device *root, int debug_level,
			 const char *msg);
void show_devs_tree(const struct device *dev, int debug_level, int depth);
void show_devs_subtree(struct device *root, int debug_level, const char *msg);
void show_all_devs(int debug_level, const char *msg);
void show_all_devs_tree(int debug_level, const char *msg);
void show_one_resource(int debug_level, struct device *dev,
		       struct resource *resource, const char *comment);
void show_all_devs_resources(int debug_level, const char *msg);

/* Rounding for boundaries.
 * Due to some chip bugs, go ahead and round IO to 16
 */
#define DEVICE_IO_ALIGN 16
#define DEVICE_MEM_ALIGN 4096

extern struct device_operations default_dev_ops_root;
void pci_domain_read_resources(struct device *dev);
void pci_domain_scan_bus(struct device *dev);

void fixed_mem_resource(struct device *dev, unsigned long index,
		  unsigned long basek, unsigned long sizek, unsigned long type);

void mmconf_resource_init(struct resource *res, resource_t base, int buses);
void mmconf_resource(struct device *dev, unsigned long index);

/* It is the caller's responsibility to adjust regions such that ram_resource()
 * and mmio_resource() do not overlap.
 */
#define ram_resource(dev, idx, basek, sizek) \
	fixed_mem_resource(dev, idx, basek, sizek, IORESOURCE_CACHEABLE)

#define reserved_ram_resource(dev, idx, basek, sizek) \
	fixed_mem_resource(dev, idx, basek, sizek, IORESOURCE_CACHEABLE \
		| IORESOURCE_RESERVE)

#define bad_ram_resource(dev, idx, basek, sizek) \
	reserved_ram_resource((dev), (idx), (basek), (sizek))

#define uma_resource(dev, idx, basek, sizek) \
	fixed_mem_resource(dev, idx, basek, sizek, IORESOURCE_RESERVE)

#define mmio_resource(dev, idx, basek, sizek) \
	fixed_mem_resource(dev, idx, basek, sizek, IORESOURCE_RESERVE)

void tolm_test(void *gp, struct device *dev, struct resource *new);
u32 find_pci_tolm(struct bus *bus);

DEVTREE_CONST struct device *dev_find_slot(unsigned int bus,
						unsigned int devfn);
DEVTREE_CONST struct device *dev_find_next_pci_device(
				DEVTREE_CONST struct device *previous_dev);
DEVTREE_CONST struct device *dev_find_slot_on_smbus(unsigned int bus,
							unsigned int addr);
DEVTREE_CONST struct device *dev_find_slot_pnp(u16 port, u16 device);
DEVTREE_CONST struct device *dev_bus_each_child(const struct bus *parent,
				DEVTREE_CONST struct device *prev_child);

DEVTREE_CONST struct device *pcidev_path_behind(const struct bus *parent,
		pci_devfn_t devfn);
DEVTREE_CONST struct device *pcidev_path_on_root(pci_devfn_t devfn);
DEVTREE_CONST struct device *pcidev_on_root(uint8_t dev, uint8_t fn);

void scan_smbus(struct device *bus);
void scan_generic_bus(struct device *bus);
void scan_static_bus(struct device *bus);
void scan_lpc_bus(struct device *bus);
void scan_usb_bus(struct device *bus);

#endif /* !defined(__ROMCC__) */

#endif /* DEVICE_H */
