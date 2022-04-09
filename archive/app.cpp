#include <string.h>
#include "app.h"
#include "mySerial.h"
#include "libusb.h"

static void print_device(libusb_device *dev);
static void print_configuration(struct libusb_config_descriptor *config);
static void print_bos(libusb_device_handle *handle);
static void print_interface(const struct libusb_interface *interface);
static void print_endpoint_comp(const struct libusb_ss_endpoint_companion_descriptor *ep_comp);
static void print_endpoint(const struct libusb_endpoint_descriptor *endpoint);
static void print_2_0_ext_cap(struct libusb_usb_2_0_extension_descriptor *usb_2_0_ext_cap);
static void print_ss_usb_cap(struct libusb_ss_usb_device_capability_descriptor *ss_usb_cap);

static void get_endpoints(libusb_device *device, uint8_t *endpoint_in, uint8_t *endpoint_out);

static int verbose = 0;

int main ()
{

}

int main ()
{
    printf ("Test App: %s\n", APP_MSG);
    //runtest();

    uint8_t endpointIn, endpointOut;

    libusb_device **devs;
    libusb_context *ctx = NULL;
    libusb_device_handle *dev_handle; //a device handle
    int r; ssize_t cnt;
    r = libusb_init(&ctx);
    if (r < 0) {
        printf("error\n");
        return 0;
    }
    libusb_set_debug(ctx, 3);
    cnt = libusb_get_device_list(ctx, &devs);
    if (cnt < 0) {
        printf("get device error\n");
    }
    /*for (int i = 0; i < cnt; i++) {
        print_device(devs[i]);
    }*/

    //print_device(devs[7]);

    //dev_handle = libusb_open_device_with_vid_pid(ctx, 0403, 6001);

    r = libusb_open(devs[7], &dev_handle);
    if (r < 0) {
        printf("Error\n");
        return 0;
    }

    if(dev_handle == NULL)
	    printf("Cannot open device\n");
	else
	    printf("Device Opened\n");

    printf("Freeing device list..\n");
    libusb_free_device_list(devs, 1); //free the list, unref the devices in it    
    
    char teststring [] = "teststring";
    int actual;

    if(libusb_kernel_driver_active(dev_handle, 0) == 1) 
    {   //find out if kernel driver is attached
        printf("Kernel Driver Active\n");
        if(libusb_detach_kernel_driver(dev_handle, 0) == 0) //detach it
            printf("Kernel Driver Detached!\n");
    }

    r = libusb_claim_interface(dev_handle, 0); //claim interface 0 (the first) of device (mine had jsut 1)
    if(r < 0) 
    {
        printf("Cannot Claim Interface\n");
        return 1;
    }
    printf("Claimed Interface\n");

    printf("Data sent: %s\n", teststring);

    printf("Writing data...\n");

    get_endpoints(devs[7], &endpointIn, &endpointOut);

    r = libusb_bulk_transfer(dev_handle, (/*2 | LIBUSB_ENDPOINT_OUT*/endpointOut), \
        teststring, strlen(teststring), &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129

    if (r == 0 && actual == strlen(teststring))
        printf("Writing successful!\n");
    else
        printf("Writing Error, r = %d, actual = %d\n", r, actual);
    
    //listen
    int cptRecv = 0; char recvteststring [32];
    while (cptRecv <= 10)
    {
        memset(recvteststring, 0, sizeof(recvteststring));
        r = libusb_bulk_transfer(dev_handle, (/*2 | LIBUSB_ENDPOINT_IN | LIBUSB_TRANSFER_TYPE_BULK_STREAM*/endpointIn), \
        recvteststring, strlen(recvteststring), &actual, 0); //my device's out endpoint was 2, found with trial- the device had 2 endpoints: 2 and 129        
        printf("%d, res = %s, Received String = %s, size %d\n", cptRecv, libusb_error_name(r), recvteststring, actual);
        cptRecv++;
    }

    r = libusb_release_interface(dev_handle, 0); //release the claimed interface
    if(r!=0) 
    {
        printf("Cannot Release Interface\n");
        return 1;
    }
    printf("Released Interface\n");
    libusb_close(dev_handle);

    libusb_exit(ctx); //close the session
    
    return 0;
}

static void get_endpoints(libusb_device *device, uint8_t *endpoint_in, uint8_t *endpoint_out) 
{
	int i = 0, j = 0, k = 0;

	struct libusb_device_descriptor desc = {0};
	struct libusb_config_descriptor *config = NULL;
	const struct libusb_endpoint_descriptor *endpoint = NULL;

	libusb_get_device_descriptor(device, &desc);
	libusb_get_config_descriptor(device, 0, &config);

	for(i = 0; i < config->bNumInterfaces; ++i) {
		for(j = 0; j < config->interface[i].num_altsetting; ++j) {
			for(k = 0; k < config->interface[i].altsetting[j].bNumEndpoints; ++k) {
				endpoint = &config->interface[i].altsetting[j].endpoint[k];

				if ((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) & LIBUSB_TRANSFER_TYPE_INTERRUPT) {
					if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
						*endpoint_in = endpoint->bEndpointAddress;
					} else {
						*endpoint_out = endpoint->bEndpointAddress;
					}
				}
			}
		}
	}

	libusb_free_config_descriptor(config);
	return;
}

static void print_2_0_ext_cap(struct libusb_usb_2_0_extension_descriptor *usb_2_0_ext_cap)
{
	printf("    USB 2.0 Extension Capabilities:\n");
	printf("      bDevCapabilityType:    %u\n", usb_2_0_ext_cap->bDevCapabilityType);
	printf("      bmAttributes:          %08xh\n", usb_2_0_ext_cap->bmAttributes);
}

static void print_ss_usb_cap(struct libusb_ss_usb_device_capability_descriptor *ss_usb_cap)
{
	printf("    USB 3.0 Capabilities:\n");
	printf("      bDevCapabilityType:    %u\n", ss_usb_cap->bDevCapabilityType);
	printf("      bmAttributes:          %02xh\n", ss_usb_cap->bmAttributes);
	printf("      wSpeedSupported:       %u\n", ss_usb_cap->wSpeedSupported);
	printf("      bFunctionalitySupport: %u\n", ss_usb_cap->bFunctionalitySupport);
	printf("      bU1devExitLat:         %u\n", ss_usb_cap->bU1DevExitLat);
	printf("      bU2devExitLat:         %u\n", ss_usb_cap->bU2DevExitLat);
}

static void print_altsetting(const struct libusb_interface_descriptor *interface)
{
	uint8_t i;

	printf("    Interface:\n");
	printf("      bInterfaceNumber:      %u\n", interface->bInterfaceNumber);
	printf("      bAlternateSetting:     %u\n", interface->bAlternateSetting);
	printf("      bNumEndpoints:         %u\n", interface->bNumEndpoints);
	printf("      bInterfaceClass:       %u\n", interface->bInterfaceClass);
	printf("      bInterfaceSubClass:    %u\n", interface->bInterfaceSubClass);
	printf("      bInterfaceProtocol:    %u\n", interface->bInterfaceProtocol);
	printf("      iInterface:            %u\n", interface->iInterface);

	for (i = 0; i < interface->bNumEndpoints; i++)
		print_endpoint(&interface->endpoint[i]);
}

static void print_endpoint_comp(const struct libusb_ss_endpoint_companion_descriptor *ep_comp)
{
	printf("      USB 3.0 Endpoint Companion:\n");
	printf("        bMaxBurst:           %u\n", ep_comp->bMaxBurst);
	printf("        bmAttributes:        %02xh\n", ep_comp->bmAttributes);
	printf("        wBytesPerInterval:   %u\n", ep_comp->wBytesPerInterval);
}

static void print_endpoint(const struct libusb_endpoint_descriptor *endpoint)
{
	int i, ret;

	printf("      Endpoint:\n");
	printf("        bEndpointAddress:    %02xh\n", endpoint->bEndpointAddress);
	printf("        bmAttributes:        %02xh\n", endpoint->bmAttributes);
	printf("        wMaxPacketSize:      %u\n", endpoint->wMaxPacketSize);
	printf("        bInterval:           %u\n", endpoint->bInterval);
	printf("        bRefresh:            %u\n", endpoint->bRefresh);
	printf("        bSynchAddress:       %u\n", endpoint->bSynchAddress);

	for (i = 0; i < endpoint->extra_length;) {
		if (LIBUSB_DT_SS_ENDPOINT_COMPANION == endpoint->extra[i + 1]) {
			struct libusb_ss_endpoint_companion_descriptor *ep_comp;

			ret = libusb_get_ss_endpoint_companion_descriptor(NULL, endpoint, &ep_comp);
			if (LIBUSB_SUCCESS != ret)
				continue;

			print_endpoint_comp(ep_comp);

			libusb_free_ss_endpoint_companion_descriptor(ep_comp);
		}

		i += endpoint->extra[i];
	}
}

static void print_device(libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	libusb_device_handle *handle = NULL;
	char string[256];
	int ret;
	uint8_t i;

	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret < 0) {
		fprintf(stderr, "failed to get device descriptor");
		return;
	}

	printf("Dev (bus %u, device %u): %04X - %04X\n",
	       libusb_get_bus_number(dev), libusb_get_device_address(dev),
	       desc.idVendor, desc.idProduct);

	ret = libusb_open(dev, &handle);
	if (LIBUSB_SUCCESS == ret) {
		if (desc.iManufacturer) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, string, sizeof(string));
			if (ret > 0)
				printf("  Manufacturer:              %s\n", string);
		}

		if (desc.iProduct) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, string, sizeof(string));
			if (ret > 0)
				printf("  Product:                   %s\n", string);
		}

		if (desc.iSerialNumber && verbose) {
			ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, string, sizeof(string));
			if (ret > 0)
				printf("  Serial Number:             %s\n", string);
		}
	}

	if (verbose) {
		for (i = 0; i < desc.bNumConfigurations; i++) {
			struct libusb_config_descriptor *config;

			ret = libusb_get_config_descriptor(dev, i, &config);
			if (LIBUSB_SUCCESS != ret) {
				printf("  Couldn't retrieve descriptors\n");
				continue;
			}

			print_configuration(config);

			libusb_free_config_descriptor(config);
		}

		if (handle && desc.bcdUSB >= 0x0201)
			print_bos(handle);
	}

	if (handle)
		libusb_close(handle);
}

static void print_interface(const struct libusb_interface *interface)
{
	int i;

	for (i = 0; i < interface->num_altsetting; i++)
		print_altsetting(&interface->altsetting[i]);
}

static void print_configuration(struct libusb_config_descriptor *config)
{
	uint8_t i;

	printf("  Configuration:\n");
	printf("    wTotalLength:            %u\n", config->wTotalLength);
	printf("    bNumInterfaces:          %u\n", config->bNumInterfaces);
	printf("    bConfigurationValue:     %u\n", config->bConfigurationValue);
	printf("    iConfiguration:          %u\n", config->iConfiguration);
	printf("    bmAttributes:            %02xh\n", config->bmAttributes);
	printf("    MaxPower:                %u\n", config->MaxPower);

	for (i = 0; i < config->bNumInterfaces; i++)
		print_interface(&config->interface[i]);
}

static void print_bos(libusb_device_handle *handle)
{
	struct libusb_bos_descriptor *bos;
	uint8_t i;
	int ret;

	ret = libusb_get_bos_descriptor(handle, &bos);
	if (ret < 0)
		return;

	printf("  Binary Object Store (BOS):\n");
	printf("    wTotalLength:            %u\n", bos->wTotalLength);
	printf("    bNumDeviceCaps:          %u\n", bos->bNumDeviceCaps);

	for (i = 0; i < bos->bNumDeviceCaps; i++) {
		struct libusb_bos_dev_capability_descriptor *dev_cap = bos->dev_capability[i];

		if (dev_cap->bDevCapabilityType == LIBUSB_BT_USB_2_0_EXTENSION) {
			struct libusb_usb_2_0_extension_descriptor *usb_2_0_extension;

			ret = libusb_get_usb_2_0_extension_descriptor(NULL, dev_cap, &usb_2_0_extension);
			if (ret < 0)
				return;

			print_2_0_ext_cap(usb_2_0_extension);
			libusb_free_usb_2_0_extension_descriptor(usb_2_0_extension);
		} else if (dev_cap->bDevCapabilityType == LIBUSB_BT_SS_USB_DEVICE_CAPABILITY) {
			struct libusb_ss_usb_device_capability_descriptor *ss_dev_cap;

			ret = libusb_get_ss_usb_device_capability_descriptor(NULL, dev_cap, &ss_dev_cap);
			if (ret < 0)
				return;

			print_ss_usb_cap(ss_dev_cap);
			libusb_free_ss_usb_device_capability_descriptor(ss_dev_cap);
		}
	}

	libusb_free_bos_descriptor(bos);
}

//  libusb_device_handle *handle;
// 	libusb_device *dev;
//     uint8_t bus, port_path[8];
//     int i, j, k, r;
//     printf("Opening device %04X:%04X...\n", vid, pid);
// 	handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
//     dev = libusb_get_device(handle);
// 	bus = libusb_get_bus_number(dev);
//     struct libusb_device_descriptor dev_desc;
// 	const char* speed_name[5] = { "Unknown", "1.5 Mbit/s (USB LowSpeed)", "12 Mbit/s (USB FullSpeed)",
// 		"480 Mbit/s (USB HighSpeed)", "5000 Mbit/s (USB SuperSpeed)"};
// 	char string[128];
// 	uint8_t string_index[3];	// indexes of the string descriptors
// 	uint8_t endpoint_in = 0, endpoint_out = 0;	// default IN and OUT endpoints
// 	printf("Opening device %04X:%04X...\n", vid, pid);
// 	handle = libusb_open_device_with_vid_pid(NULL, vid, pid);
// 	if (handle == NULL) {
// 		perr("  Failed.\n");
// 		return -1;
// 	}
// 	dev = libusb_get_device(handle);
// 	bus = libusb_get_bus_number(dev);

//     r = libusb_get_port_numbers(dev, port_path, sizeof(port_path));
//     if (r > 0) 
//     {
//         printf("\nDevice properties:\n");
//         printf("        bus number: %d\n", bus);
//         printf("         port path: %d", port_path[0]);
//         for (i=1; i<r; i++) 
//         {
//             printf("->%d", port_path[i]);
//         }
//         printf(" (from root hub)\n");
//     }
//     r = libusb_get_device_speed(dev);
//     if ((r<0) || (r>4)) r=0;
//     printf("             speed: %s\n", speed_name[r]);