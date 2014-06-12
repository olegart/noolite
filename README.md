##noolite
=======

Драйверы приёмника и передатчика системы «умного дома» nooLite (http://www.noo.com.by/sistema-noolite.html) под Linux.  
Основано на проектах https://github.com/ermolenkom/noolite и https://github.com/vvzvlad/noolite/  
Дополнено (nooliterx превращен в полноценную утилиту с управлением из командной строки и конфигурационного файла, режимом демона и т.п.), причесано, сборка переведена под autoconf/automake  

##Установка
Помимо обычного окружения для сборки ПО, устанавливаем библиотеку libusb-1.0 (нужна для работы с HID-устройством)

  `sudo apt-get install libusb-1.0-0 libusb-1.0-0-dev`
или
  `sudo yum install libusb1 libusb1-devel`

(конкретика зависит от дистрибутива)
  
Скачиваем исходники  
  `wget https://github.com/olegart/noolite/archive/master.zip`  
  `unzip master.zip`  
  `cd noolite-master`  
  `./configure && make && make install`  
  
Инструкции по использованию — в файле doc/README

Makefile и скрипт автозапуска для OpenWRT — в папке openwrt
