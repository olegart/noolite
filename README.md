#noolite
=======
За основу был взят вот этот проект: https://github.com/ermolenkom/noolite

Отличия: 
Проект избавлен от api windows-версии
  
  
##Инструкция.
Устанавливаем библиотеку libusb (нужна для работы с HID-устройством)
  `sudo apt-get install libusb-1.0-0 libusb-1.0-0-dev`
  
Скачиваем  
  `svn co https://github.com/vvzvlad/noolite.git`
  `cd noolite/trunk`
  `make`
  

###Использование:

запустить 

  `nooliterx`
  
Для автоматического запуска в Ubuntu на уровнях 2,3,4,5 в режиме демона

   `sudo cp majordomo_nooliterx.conf /etc/init`
   
Ручной запуск
 `start majordomo_nooliterx`
Ручной останов
 `stop majordomo_nooliterx`

###Применение в другой системе "Умный Дом":

При получении нажатия драйвер запрашивает веб-страницу на локальном сервере. Необходимо заменить строчку

`wget http://localhost/objects/?script=switchNooLitePress\\&buf0=%i\\&buf1=%i\\&buf2=%i\\&buf3=%i\\&buf4=%i\\&buf5=%i\\&buf6=%i\\&buf7=%i -O /dev/null`

на свой вариант.







