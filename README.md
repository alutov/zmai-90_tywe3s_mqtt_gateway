# Альтернативная прошивка ZMAi-90 (TYWE3S+V9821/V9821S) с поддержкой Mqtt.
![PROJECT_PHOTO](https://github.com/alutov/zmai-90_mqtt_gateway/blob/main/jpg/web1.jpg)

RUS | [ENG](https://translate.google.com/translate?hl=ru&sl=ru&tl=en&u=https%3A%2F%2Fgithub.com%2Falutov%2Fzmai-90_mqtt_gateway%2Fblob%2Fmaster%2FREADME.md)

#### Текущая версия 2024.02.05.
* 2024.02.05. При старте шлюз сканируются все WIFI AP с требуемыми параметрами и выбирается AP с лучшим уровнем. Актуально для меш систем. Добавлен выбор режима WIFI 802.11b/g/n в настройках.
* 2023.08.04. Изменены имена объектов в Home Assistant Mqtt Discovery для совместимости с версией 2023.8.0.

## 1. Возможности
&emsp; В Mqtt выводится 4 основных измеренных параметра: напряжение, ток, мощность и потребленная энергия. Вычисляется полная мощность и коэффициент мощности. Для синхронизации показаний энергии со счетчиком коммерческого учета предусмотрена опция Еnergy offset. Значение из этого поля добавляется(или вычитается) к текущим показаниям при выводе в Mqtt. Доступно управление реле, как длительным нажатием кнопки, так и по Mqtt. Смысл отключения электроэнергии во всем доме или квартире представляю слабо, а потому реле включается при каждой перезагрузке TYWE3S. Поддерживается два режима управления по Mqtt. Если опция "Mqtt "OFF" switch to "ON" after 30sec delay" выключена, при поступлении команды выключения реле выключается до прихода команды включения или перезагрузки ZMAi-90. Если при этом пропадает питание на сервере умного дома или роутере, дистанционно включить реле уже не получится. Придется ждать пропадания напряжения на входе ZMAi-90 или же локально нажимать кнопку. Если эта опция включена, что команда выключения выключает реле только на 30 секунд, после чего реле опять включается. Это дает возможность дистанционного сброса всех устройств в доме или квартире путем снятия напряжения. Поддерживается Home Assistant Mqtt Discovery. Прошивка рассчитана на работу с новой ревизией (у меня 2034) [ZMAi-90, в которой TYWE3S/ESP8266](https://aliexpress.ru/item/4000630131830.html?spm=a2g0s.9042311.0.0.264d33edpcxOl3&_ga=2.3591289.876087626.1614398092-1873776317.1607235158&sku_id=10000006206237370) использует для управления устройством только последовательный порт. Поддерживаются MCU V9821 или V9821S, выбирается в настройках. ZMAI-90 может быть собран на [V9821S в паре с WB3S](https://aliexpress.ru/item/4001053795800.html?spm=a2g0s.9042311.0.0.258d33ed9zW2VC). Для работы с этой прошивкой WB3S нужно заменить на ESP12. Файл fzmai.bin в папке build это уже собранный бинарник для TYWE3S (esp8266) с памятью 1 Мбайт и прошивается одним файлом с адреса 0x0000 на чистую TYWE3S/ESP8266. Вместо него также можно использовать три стандартных файла для перепрошивки: bootloader.bin (адрес 0x000),  partitions.bin (адрес 0x8000) и zmai.bin (адрес 0x10000). Файл zmai.bin можно также использовать для обновления прошивки через web интерфейс. Перед прошивкой нужно замкнуть вход RST(47 нога) MCU V9821 на корпус, чтобы он не мешал прошивке. Подробнее [здесь](https://kvvhost.ru/2020/04/18/zmai-90-tasmota/). После прошивки перемычку нужно снять. Затем нужно создать гостевую сеть Wi-Fi в роутере с ssid "zmai" и паролем "12345678", подождать, пока TYWE3S не подключится к нему, ввести TYWE3S IP-адрес в веб-браузере и во вкладке Setting установить остальные параметры. После чего гостевая сеть больше не нужна. TYWE3S будет пытаться подключиться к сети "zmai" только при недоступности основной сети, например, при неправильном пароле. Если не удается подключиться и к гостевой сети, TYWE3S перезагружается. Предусмотрена возможность подключения к одному MQTT серверу нескольких ZMAi-90. Для этого нужно в каждом устройстве установить свой ZMAi-90 Number. Устройство с номером 0 будет писать в топик zmai/..., с номером 1 - zmai1/... и т.д.
<br>
![PROJECT_PHOTO](https://github.com/alutov/zmai-90_mqtt_gateway/blob/main/jpg/web2.jpg)
<br>
# 2. Сборка проекта
&emsp; Для сборки бинарных файлов использован  [ESP8266_RTOS_SDK (IDF Style) версии 3.2](https://codeload.github.com/espressif/ESP8266_RTOS_SDK/zip/v3.2) c [описанием](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/) и [toolchain 5.2.0](https://dl.espressif.com/dl/xtensa-lx106-elf-win32-1.22.0-92-g8facf4c-5.2.0.tar.gz). Более свежую библиотеку esp-mqtt взял [тут](https://github.com/looi/ESP8266_RTOS_SDK).<br>
