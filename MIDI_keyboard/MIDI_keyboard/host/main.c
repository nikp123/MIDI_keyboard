#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb.h"

#define USB_LED_OFF 0
#define USB_LED_ON  1

static int usbGetDescriptorString(usb_dev_handle *dev, int index, int langid, 
                                  char *buf, int buflen)
								  {
   char buffer[256];
   int rval, i;
  
   // Создание стандартного запроса GET_DESCRIPTOR, тип string,
   //  с указанием индекса (например dev->iProduct):
   rval = usb_control_msg(dev, USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
						  USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, 
						  langid, buffer, sizeof(buffer), 1000);
   
    if(rval < 0) // если отрицательное число, то ошибка и возврат
      return rval;
   
   // rval должно содержать количество прочитанных байт, однако в buffer[0]
   // содержится реальный размер ответа
   if((unsigned char)buffer[0] < rval)
      rval = (unsigned char)buffer[0]; // string короче, чем количество прочитанных байт
   
   if(buffer[1] != USB_DT_STRING)      // во втором байте содержится тип данных
      return 0;                        // ошибочный тип данных
   
   // мы работаем с форматом UTF-16LE, так что в действительности символов наполовину
   // меньше rval, и индекс 0 не учитывается
   rval /= 2;
   
   /* Алгоритм преобразования (с потерями, т. е. фильтрацией) в ISO Latin1 */
   for(i = 1; i < rval && i < buflen; i++)
   {
      if(buffer[2 * i + 1] == 0)
         buf[i-1] = buffer[2 * i];
      else
         buf[i-1] = '?'; /* вне диапазона ISO Latin1 */
   }
   buf[i-1] = 0;
   
   return i-1;
}

static usb_dev_handle * usbOpenDevice(int vendor, char *vendorName, 
                                      int product, char *productName)
									  {
   struct usb_bus *bus;
   struct usb_device *dev;
   char devVendor[256], devProduct[256];
  
   usb_dev_handle * handle = NULL;
  
   usb_init();
   usb_find_busses();
   usb_find_devices();
  
   for(bus=usb_get_busses(); bus; bus=bus->next)
   {
      for(dev=bus->devices; dev; dev=dev->next)
      {
         if(dev->descriptor.idVendor != vendor ||
            dev->descriptor.idProduct != product)
            continue;
  
         /* Чтобы запросить строки, нужно "открыть" устройство USB */
         if(!(handle = usb_open(dev)))
         {
            fprintf(stderr,
                    "Warning: cannot open USB device: %sn",
                    usb_strerror());
            continue;
         }
  
         /* получение vendor name */
         if(usbGetDescriptorString(handle, dev->descriptor.iManufacturer, 
                                   0x0409, devVendor, sizeof(devVendor)) < 0)
         {
            fprintf(stderr, 
                    "Warning: cannot query manufacturer for device: %sn", 
                    usb_strerror());
            usb_close(handle);
            continue;
         }
  
         /* получение product name */
         if(usbGetDescriptorString(handle, dev->descriptor.iProduct, 
                                   0x0409, devProduct, sizeof(devVendor)) < 0)
         {
            fprintf(stderr, 
                    "Warning: cannot query product for device: %sn", 
                    usb_strerror());
            usb_close(handle);
            continue;
         }
  
         if(strcmp(devVendor, vendorName) == 0 && 
            strcmp(devProduct, productName) == 0)
            return handle;
         else
            usb_close(handle);
      }
    }
  
    return NULL;
}

int main(int argc, char **argv)
{
   usb_dev_handle *handle = NULL;
   int nBytes = 0;
   char buffer[256];
  
   if(argc < 2)
   {
      printf("Usage:\n");
      printf("usbtext.exe on\n");
      printf("usbtext.exe off\n");
      exit(1);
   }
  
   handle = usbOpenDevice(0x16C0, "SS", 0x05DC, "ss_midi");
  
   if(handle == NULL)
   {
      fprintf(stderr, "Could not find USB device!n");
      exit(1);
   }
   
   if(strcmp(argv[1], "on") == 0)
   {
      nBytes = usb_control_msg(handle, 
                               USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                               USB_LED_ON, 0, 0, (char *)buffer, sizeof(buffer), 5000);
   }
   else if(strcmp(argv[1], "off") == 0)
   {
      nBytes = usb_control_msg(handle, 
                               USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
                               USB_LED_OFF, 0, 0, (char *)buffer, sizeof(buffer), 5000);
   }
  
   if(nBytes < 0)
      fprintf(stderr, "USB error: %sn", usb_strerror());
  
   usb_close(handle);
  
   return 0;
}