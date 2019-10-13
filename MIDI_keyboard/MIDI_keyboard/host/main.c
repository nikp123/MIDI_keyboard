static int usbGetDescriptorString(usb_dev_handle *dev, int index, int langid, 
                                  char *buf, int buflen)
{
   char buffer[256];
   int rval, i;
  
   // Создание стандартного запроса GET_DESCRIPTOR, тип string,
   //  с указанием индекса (например dev->iProduct):
   rval = usb_control_msg(dev, 
      USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_ENDPOINT_IN, 
      USB_REQ_GET_DESCRIPTOR, (USB_DT_STRING << 8) + index, langid, 
      buffer, sizeof(buffer), 1000);
   
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