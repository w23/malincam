# Всякое знание про камеры на малине

## TODO ПРОЧИТАЙ
- https://www.marcusfolkesson.se/blog/v4l2-and-media-controller/
- https://kakaroto.homelinux.net/2012/09/uvc-h264-encoding-cameras-support-in-gstreamer/

## Аппаратчики
- У всех малин CS2 2lane, вроде как это 720p60 || 1080p30 макс
  - pi5 и cm4 имеют все 4, можно 2x больше фпс
- Енкодер h264 макс 720p60, 1080p30
- TODO Енкодер jpeg аппаратный макс ?
- Pi0/Pi0W == Pi1/2?
- Pi0 2 == Pi3
- pi4 умеет в hevc аппаратно туда-сюда

## Софтня
- Есть старый MMAL проприетарный, для 64бита косячный/недоделанный, не используй
- Есть новый V4L2 стандартный, в него вынесено почти всё, что умел ммал (но вроде не всё?)
- Есть libcamera, оборачивающий всё это в какой-то якобы более удобный интерфейс, НО:
  - Кресты, очень медленно собирается на Pi0 2.
  - Делает всякое софтом: jpeg, коррекцию изображений (автояркость, белизна -- не нужно, можно вырубить)
  - Примеры из libcamera-apps абсолюно долбанутые на всю голову крестами, кто так пишет эмбедед

## Разное
- Дока подробная по кишкам софта и камер: https://datasheets.raspberrypi.com/camera/raspberry-pi-camera-guide.pdf
- https://www.raspberrypi.com/documentation/computers/camera_software.html
- Статус разных подсистем (на 2021, ещё релевантно; есть ссылки дальше) https://forums.raspberrypi.com/viewtopic.php?t=317511

### v4l2
- Можно даже дружить с OpenGL через DMABUF
    - TBD пример (есть)
- Рассыпает функциональность на пачку /dev/video* устройств
    - например, /dev/video0 это вход камеры
    - /dev/video1 какие-то метаданные
    - /dev/video10-12,... декодинг h264, енкодинг h264, процессинг/ISP блоки, ... jpeg, ...
Кормится рассыпуха тупо прокидыванием dmabuf fd друг в друга.

#### Доки
https://www.linuxtv.org/downloads/v4l-dvb-apis-new/userspace-api/v4l/ext-ctrls-image-source.html
https://www.linuxtv.org/downloads/v4l-dvb-apis-new/userspace-api/v4l/control.html
https://www.linuxtv.org/downloads/v4l-dvb-apis-new/userspace-api/v4l/vidioc-subdev-g-selection.html
https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/vidioc-expbuf.html
https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dev-mem2mem.html
https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dmabuf.html
https://www.kernel.org/doc/html/v5.11/userspace-api/media/drivers/uvcvideo.html
- stateful (h264), держит стейт, просто кормить: https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dev-decoder.html
- stateless (h265, >=pi4), сложно кормить (use ffmpeg): https://www.kernel.org/doc/html/latest/userspace-api/media/v4l/dev-stateless-decoder.html

#### Примеры
- h264: https://github.com/raspberrypi/libcamera-apps/blob/main/encoder/h264_encoder.cpp
- v4l2 isp: https://forums.raspberrypi.com/viewtopic.php?t=352876
- dmabuf в gl: https://forums.raspberrypi.com/viewtopic.php?t=281296
    - https://github.com/6by9/yavta/blob/master/yavta.c#L842
    - https://github.com/6by9/drm_mmal/blob/x11/drm_mmal.c#L476
    - https://github.com/6by9/drm_mmal/blob/x11/drm_mmal.c#L985
    - https://github.com/6by9/yavta/blob/master/yavta.c
- https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/v4l2_context.c#L132
- https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/v4l2_buffers.c#L512
- https://github.com/FFmpeg/FFmpeg/blob/master/libavcodec/v4l2_context.c#L391
- https://github.com/GStreamer/gst-plugins-good/blob/master/sys/v4l2/gstv4l2object.c
- dmabuf into gl:
    - https://github.com/monolith-jspark/rpi_v4l2dma_opengl_test/
- https://github.com/ArduCAM/MIPI_Camera/blob/master/RPI/raw_callback.c
- https://github.com/raspberrypi/linux/blob/rpi-4.19.y/drivers/staging/vc04_services/bcm2835-codec/bcm2835-v4l2-codec.c
- https://github.com/raspberrypi/linux/blob/rpi-5.10.y/drivers/staging/vc04_services/bcm2835-isp/bcm2835-v4l2-isp.c
- эффективное декодирование ffmpeg в kms: https://github.com/jc-kynesim/hello_drmprime

## Showmewebcam OS
- Давно не обновлялась, старая, на старых ядрах.
- Старый форк uvc-gadget
- Но вроде как всё равно уже на v4l2. Не на libcamera во всяком случае.
- cameractl в её составе может в фиксированный руками баланс белого и iso. Можно выставить в нормальный.
- Не может в 720p60, т.к. в скриптах и в uvc-gadget забито гвоздями макс 30фпс. Поменять трудно, т.к. squashfs readonly.
    - только пересобирать всё?
- Нет сети, только ttyUSB, ничего нельзя просто поменять.
- UVC OTG устройство настраивается шелл-скриптом

## Raspberry Pi OS
- Сейчас самый свежий bookworm
- Можно руками повторить примерно то же, что и showmewebcam, но:
  - uvc-gadget старый не работает, новый не работает через v4l2 напрямую, только через libcamera
  - libcamera пакует mjpeg софтом, что в 60фпс упирается в 100% проц
  - почему-то постоянно сыпятся ошибки uvc на хосте, видео дёргается на всём, кроме явного mjpeg
  - mjpeg выглядит гладко, но зелено (?!)
  - по сети всё стабильно с h264 720p60, но общая задержка > 500мс.

## Примеры

- RTSP libcamera-apps: https://forums.raspberrypi.com/viewtopic.php?t=354028

### ustreamer
https://github.com/pikvm/ustreamer/tree/master
Вроде как:
- использует v4l2
- аппаратно жмёт jpeg

### uvc-gadget
old+working+showmewebcam: https://github.com/peterbay/uvc-gadget
new+libcamera-meh: https://gitlab.freedesktop.org/camera/uvc-gadget/

### cameractl
Крутилки и их значения

### https://github.com/ayufan/camera-streamer
Как это https://github.com/ayufan/camera-streamer/blob/main/docs/v4l2-isp-mode.md

### https://github.com/kinweilee/v4l2-mmal-uvc

# План работ
- [ ] Попробуй руками потрогать за v4l2?
- [ ] Попробуй к libcamera прикрутить аппаратный mjpeg?
- [ ] Протрейсь usb-otg, почему там ошибки

# ETC
- https://qengineering.eu/protect-the-raspberry-pi-4-sd-flashcard.html

- DMABUF export, kernel patching, etc: https://forums.raspberrypi.com/viewtopic.php?t=291940
- Yet another V4L2 to OpenGL via DMABUF thread: https://forums.raspberrypi.com/viewtopic.php?f=68&t=281296&hilit=yavta
