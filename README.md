#noolite
=======
Драйверы приёмника и передатчика системы «умного дома» nooLite (http://www.noo.com.by/sistema-noolite.html) под Linux.  
Основано на проектах https://github.com/ermolenkom/noolite и https://github.com/vvzvlad/noolite/

Изменения: 
* nooliterx: добавлены опции командной строки (установка таймаута, выполняемая команда)
* nooliterx: в выполняемую команду можно передавать любое количество их имеющихся аргументов в любом порядке
* nooliterx: добавлен режим демона
* nooliterx: вся выдача переведена на английский
* nooliterx: код отформатирован в едином стиле
* nooliterx: несколько малосущественных фиксов
* nooliterxcfg: поправлено число каналов в справке ([0..8] → [0..64])
  
  
##Инструкция
Устанавливаем библиотеку libusb (нужна для работы с HID-устройством)

  `sudo apt-get install libusb-1.0-0 libusb-1.0-0-dev`

  `sudo yum install libusb1 libusb1-devel`

(зависит от дистрибутива)
  
Скачиваем исходники  
  `svn co https://github.com/olegart/noolite.git`  
  `cd noolite/trunk`  
  `make`  
  

###Использование:

######noolitepc — драйвер передатчика (PC118, PC1116, PC1132).

  `noolitepc -api -<command> <channel> [<level>|<RGB>]`
  
command — основная команда:  
--on - включить канал  
--off - выключить канал  
--sw - переключить канал (вкл → выкл, выкл → вкл)  
--set - установить уровень яркости в канале (для диммируемых блоков)  
--bind - привязать канал передатчика к силовому блоку  
--unbind - отвязать канал  
--load_preset - загрузить настройки канала  
--save_preset - сохранить настройки канала  
--stop_reg - Stop change channel  
--roll_color - Rolling color  
--sw_color - Swith color  
--sw_mode - Swith mode  
--speed_mode_sw - Swith speed change color  
--set_color - установить цвет R[0..255] G[0..255] B[0..255]  
<channel> — номер канала, [1..32]  
<level> — уровень яркости [0..100] в команде --set  
<RGB> — цвет [0..255] [0..255] [0..255] в команде --set_color  

Например:  
Установить яркость 33 % в канале 1:  
  `noolitepc -api -set_ch 1 33`  

Выключить канал 5:  
  `noolitepc -api -off 5 `  

###### nooliterx — драйвер приёмника (RX1164)  

  `nooliterx [-c command] [-t timeout] [-d] [-h]`

Параметры  
  -c command — команда, которую будет запускать драйвер при приходе сигнала от пульта (по умолчанию — вывод в консоль)  
  -t timeout — таймаут исполнения команды, в миллисекундах (по умолчанию 250)  
  -d — запустить драйвер в фоне, освободив консоль  
  -h — напечатать справку и выйти  

Примеры команд для опции -c:  
  `echo 'Status: %st Channel: %ch Command: %cm Data format: %df Data bytes: %d0 %d1 %d2 %d3'`  
Печатает в консоли в одну строчку сведения о пришедшем от пульта сигнале.  
  
  `wget http://localhost/noolight/?script=switchNooLitePress\&channel=%ch\&command=%cm`  
Передаёт скрипту на локальном веб-сервере номер канала и команды при приходе сигнала от пульта.  
  
Используемые в формировании команд переменные (подробное описание их смысла см в API RX1164 на http://www.noo.com.by/):  
  %st — статус адапрета  
  %ch — номер канала  
  %cm — номер команды  
  %df — формат данных  
  %d0...%d3 — четыре байта данных  

###### nooliterxcfg — конфигурация приёмника (RX1164)  

  `nooliterxcfg -api -<command> <channel> [<level>]`

command может быть:  
  -norm_ch - обычная работа  
  -clear_ch - очистить ячейку  
  -clearall_ch - очистить всю память  
  -reset_ch - сбросить значение калибровки USB  
  -bind_ch - привязать канал  
  -unbind_ch - отвязать канал  
  -help — напечатать справку  
  
  Номер канала должен быть в диапазоне [1...64]