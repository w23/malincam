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

# UVC and controls
## UVC controls being read
```
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_INFO(86) wIndex=[ent=2 if=0](0200) wValue=0200 wLength=1
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_PROCESSING_UNIT_ID(2) control=UVC_PU_BRIGHTNESS_CONTROL(2)
[ERR] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_PROCESSING_UNIT_ID(2) control=UVC_PU_BRIGHTNESS_CONTROL(2) not found
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_CUR(81) wIndex=[ent=0 if=0](0000) wValue=0200 wLength=1
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_INTERFACE(0) control=UVC_VC_REQUEST_ERROR_CODE_CONTROL(2)
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_INFO(86) wIndex=[ent=1 if=0](0100) wValue=0200 wLength=1
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_CAMERA_TERMINAL_ID(1) control=UVC_CT_AE_MODE_CONTROL(2)
[ERR] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_CAMERA_TERMINAL_ID(1) control=UVC_CT_AE_MODE_CONTROL(2) not found
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_CUR(81) wIndex=[ent=0 if=0](0000) wValue=0200 wLength=1
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_INTERFACE(0) control=UVC_VC_REQUEST_ERROR_CODE_CONTROL(2)
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_DEF(87) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[INF] UVC_EVENT_SETUP: bRequestType=<CI(21) bRequest=UVC_SET_CUR(01) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[ERR] uvcHandleVsInterfaceProbeCommitControl: UVC_SET_CUR not implemented (cs=1)
[INF] uvc_gadget: UVC_EVENT_DATA length=34
[ERR] processEventData: not implemented (length=34), control_selector=1
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_CUR(81) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_DEF(87) wIndex=[ent=2 if=0](0200) wValue=0200 wLength=2
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_PROCESSING_UNIT_ID(2) control=UVC_PU_BRIGHTNESS_CONTROL(2)
[ERR] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_PROCESSING_UNIT_ID(2) control=UVC_PU_BRIGHTNESS_CONTROL(2) not found
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_CUR(81) wIndex=[ent=0 if=0](0000) wValue=0200 wLength=1
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_INTERFACE(0) control=UVC_VC_REQUEST_ERROR_CODE_CONTROL(2)
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_DEF(87) wIndex=[ent=2 if=0](0200) wValue=0200 wLength=2
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_PROCESSING_UNIT_ID(2) control=UVC_PU_BRIGHTNESS_CONTROL(2)
[ERR] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_PROCESSING_UNIT_ID(2) control=UVC_PU_BRIGHTNESS_CONTROL(2) not found
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_CUR(81) wIndex=[ent=0 if=0](0000) wValue=0200 wLength=1
[INF] processEventSetup: interface=UVC_INTF_VIDEO_CONTROL(0) entity=UVC_VC_ENT_INTERFACE(0) control=UVC_VC_REQUEST_ERROR_CODE_CONTROL(2)
[INF] UVC_EVENT_SETUP: bRequestType=<CI(21) bRequest=UVC_SET_CUR(01) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[ERR] uvcHandleVsInterfaceProbeCommitControl: UVC_SET_CUR not implemented (cs=1)
[INF] uvc_gadget: UVC_EVENT_DATA length=34
[ERR] processEventData: not implemented (length=34), control_selector=1
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_MIN(82) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_MAX(83) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[INF] UVC_EVENT_SETUP: bRequestType=<CI(21) bRequest=UVC_SET_CUR(01) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[ERR] uvcHandleVsInterfaceProbeCommitControl: UVC_SET_CUR not implemented (cs=1)
...
[INF] uvc_gadget: UVC_EVENT_DATA length=34
[ERR] processEventData: not implemented (length=34), control_selector=1
[INF] UVC_EVENT_SETUP: bRequestType=>CI(a1) bRequest=UVC_GET_CUR(81) wIndex=[ent=0 if=1](0001) wValue=0100 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_PROBE_CONTROL(1)
[INF] UVC_EVENT_SETUP: bRequestType=<CI(21) bRequest=UVC_SET_CUR(01) wIndex=[ent=0 if=1](0001) wValue=0200 wLength=34
[INF] processEventSetup: interface=UVC_INTF_VIDEO_STREAMING(1) entity=UVC_VS_ENT_INTERFACE(0) control=UVC_VS_COMMIT_CONTROL(2)
[ERR] uvcHandleVsInterfaceProbeCommitControl: UVC_SET_CUR not implemented (cs=2)
[INF] uvc_gadget: UVC_EVENT_DATA length=34
```

## V4L2 controls
```
[INF] [fd=3] ctrl_ext[0]: id=(9961473)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='User Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=3] ctrl_ext[1]: id=(9963793)V4L2_CID_EXPOSURE type=V4L2_CTRL_TYPE_INTEGER name='Exposure' range=[4.+1.1028] def=1028 cur=1028 flags=00000000
[INF] [fd=3] ctrl_ext[2]: id=(9963796)V4L2_CID_HFLIP type=V4L2_CTRL_TYPE_BOOLEAN name='Horizontal Flip' range=[0.+1.1] def=0 cur=1 flags=00000400
[INF]  V4L2_CTRL_FLAG_MODIFY_LAYOUT
[INF] [fd=3] ctrl_ext[3]: id=(9963797)V4L2_CID_VFLIP type=V4L2_CTRL_TYPE_BOOLEAN name='Vertical Flip' range=[0.+1.1] def=0 cur=1 flags=00000400
[INF]  V4L2_CTRL_FLAG_MODIFY_LAYOUT
[INF] [fd=3] ctrl_ext[4]: id=(10092545)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='Camera Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=3] ctrl_ext[5]: id=(10094882)V4L2_CID_CAMERA_ORIENTATION type=V4L2_CTRL_TYPE_MENU name='Camera Orientation' range=[0.+1.2] def=2 cur=2 flags=00000004
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]   menuitem[0]: name=Front
[INF]   menuitem[1]: name=Back
[INF]   menuitem[2]: -> name=External
[INF] [fd=3] ctrl_ext[6]: id=(10094883)V4L2_CID_CAMERA_SENSOR_ROTATION type=V4L2_CTRL_TYPE_INTEGER name='Camera Sensor Rotation' range=[180.+1.180] def=180 cur=180 flags=
00000004
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF] [fd=3] ctrl_ext[7]: id=(10354689)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='Image Source Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=3] ctrl_ext[8]: id=(10356993)V4L2_CID_VBLANK type=V4L2_CTRL_TYPE_INTEGER name='Vertical Blanking' range=[60.+1.8383010] def=60 cur=60 flags=00000000
[INF] [fd=3] ctrl_ext[9]: id=(10356994)V4L2_CID_HBLANK type=V4L2_CTRL_TYPE_INTEGER name='Horizontal Blanking' range=[5332.+1.65520] def=5332 cur=5332 flags=00000000
[INF] [fd=3] ctrl_ext[10]: id=(10356995)V4L2_CID_ANALOGUE_GAIN type=V4L2_CTRL_TYPE_INTEGER name='Analogue Gain' range=[0.+1.978] def=0 cur=896 flags=00000000
[INF] [fd=3] ctrl_ext[11]: id=(10356996)V4L2_CID_TEST_PATTERN_RED type=V4L2_CTRL_TYPE_INTEGER name='Red Pixel Value' range=[0.+1.4095] def=4095 cur=4095 flags=00000000
[INF] [fd=3] ctrl_ext[12]: id=(10356997)V4L2_CID_TEST_PATTERN_GREENR type=V4L2_CTRL_TYPE_INTEGER name='Green (Red) Pixel Value' range=[0.+1.4095] def=4095 cur=4095 flags=
00000000
[INF] [fd=3] ctrl_ext[13]: id=(10356998)V4L2_CID_TEST_PATTERN_BLUE type=V4L2_CTRL_TYPE_INTEGER name='Blue Pixel Value' range=[0.+1.4095] def=4095 cur=4095 flags=00000000
[INF] [fd=3] ctrl_ext[14]: id=(10356999)V4L2_CID_TEST_PATTERN_GREENB type=V4L2_CTRL_TYPE_INTEGER name='Green (Blue) Pixel Value' range=[0.+1.4095] def=4095 cur=4095 flags
=00000000
[INF] [fd=3] ctrl_ext[15]: id=(10420225)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='Image Processing Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=3] ctrl_ext[16]: id=(10422530)V4L2_CID_PIXEL_RATE type=V4L2_CTRL_TYPE_INTEGER64 name='Pixel Rate' range=[840000000.+1.840000000] def=840000000 cur=840000000 fla
gs=00000004
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF] [fd=3] ctrl_ext[17]: id=(10422531)V4L2_CID_TEST_PATTERN type=V4L2_CTRL_TYPE_MENU name='Test Pattern' range=[0.+1.4] def=0 cur=0 flags=00000000
[INF]   menuitem[0]: -> name=Disabled
[INF]   menuitem[1]: name=Color Bars
[INF]   menuitem[2]: name=Solid Color
[INF]   menuitem[3]: name=Grey Color Bars
[INF]   menuitem[4]: name=PN9
[INF] [fd=3] ctrl_ext[18]: id=(10422533)V4L2_CID_DIGITAL_GAIN type=V4L2_CTRL_TYPE_INTEGER name='Digital Gain' range=[256.+1.65535] def=256 cur=256 flags=00000000
[INF] Total ext controls: 19


[INF] [fd=8] ctrl_ext[0]: id=(9961473)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='User Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=8] ctrl_ext[1]: id=(9963790)V4L2_CID_RED_BALANCE type=V4L2_CTRL_TYPE_INTEGER name='Red Balance' range=[1.+1.65535] def=1000 cur=3285 flags=00000020
[INF]  V4L2_CTRL_FLAG_SLIDER
[INF] [fd=8] ctrl_ext[2]: id=(9963791)V4L2_CID_BLUE_BALANCE type=V4L2_CTRL_TYPE_INTEGER name='Blue Balance' range=[1.+1.65535] def=1000 cur=1618 flags=00000020
[INF]  V4L2_CTRL_FLAG_SLIDER
[INF] [fd=8] ctrl_ext[3]: id=(9968097)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Colour Correction Matrix' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[4]: id=(9968098)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Lens Shading' range=[0.+1.255] def=0 cur=0 flags=00000300
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF]  V4L2_CTRL_FLAG_EXECUTE_ON_WRITE
[INF] [fd=8] ctrl_ext[5]: id=(9968099)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Black Level' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[6]: id=(9968100)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Green Equalisation' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[7]: id=(9968101)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Gamma' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[8]: id=(9968102)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Denoise' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[9]: id=(9968103)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Sharpen' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[10]: id=(9968104)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Defective Pixel Correction' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[11]: id=(9968105)UNKNOWN type=V4L2_CTRL_TYPE_U8 name='Colour Denoise' range=[0.+1.255] def=0 cur=0 flags=00000100
[INF]  V4L2_CTRL_FLAG_HAS_PAYLOAD
[INF] [fd=8] ctrl_ext[12]: id=(10420225)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='Image Processing Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=8] ctrl_ext[13]: id=(10422533)V4L2_CID_DIGITAL_GAIN type=V4L2_CTRL_TYPE_INTEGER name='Digital Gain' range=[1.+1.65535] def=1000 cur=1000 flags=00000000
[INF] Total ext controls: 14


[INF] [fd=13] ctrl_ext[0]: id=(10289153)UNKNOWN type=V4L2_CTRL_TYPE_CTRL_CLASS name='JPEG Compression Controls' range=[0.+0.0] def=0 cur=0 flags=00000044
[INF]  V4L2_CTRL_FLAG_READ_ONLY
[INF]  V4L2_CTRL_FLAG_WRITE_ONLY
[INF] [fd=13] ctrl_ext[1]: id=(10291459)V4L2_CID_JPEG_COMPRESSION_QUALITY type=V4L2_CTRL_TYPE_INTEGER name='Compression Quality' range=[1.+1.100] def=80 cur=80 flags=0000
0000
[INF] Total ext controls: 2


Filtered controls:
subdev/sensor:
[INF] [fd=3] ctrl_ext[1]: id=(9963793)V4L2_CID_EXPOSURE type=V4L2_CTRL_TYPE_INTEGER name='Exposure' range=[4.+1.1028] def=1028 cur=1028 flags=00000000
[INF] [fd=3] ctrl_ext[10]: id=(10356995)V4L2_CID_ANALOGUE_GAIN type=V4L2_CTRL_TYPE_INTEGER name='Analogue Gain' range=[0.+1.978] def=0 cur=896 flags=00000000
[INF] [fd=3] ctrl_ext[18]: id=(10422533)V4L2_CID_DIGITAL_GAIN type=V4L2_CTRL_TYPE_INTEGER name='Digital Gain' range=[256.+1.65535] def=256 cur=256 flags=00000000

ISP:
[INF] [fd=8] ctrl_ext[1]: id=(9963790)V4L2_CID_RED_BALANCE type=V4L2_CTRL_TYPE_INTEGER name='Red Balance' range=[1.+1.65535] def=1000 cur=3285 flags=00000020
[INF] [fd=8] ctrl_ext[2]: id=(9963791)V4L2_CID_BLUE_BALANCE type=V4L2_CTRL_TYPE_INTEGER name='Blue Balance' range=[1.+1.65535] def=1000 cur=1618 flags=00000020
[INF] [fd=8] ctrl_ext[13]: id=(10422533)V4L2_CID_DIGITAL_GAIN type=V4L2_CTRL_TYPE_INTEGER name='Digital Gain' range=[1.+1.65535] def=1000 cur=1000 flags=00000000
[INF] Total ext controls: 14

Encoder:
[INF] [fd=13] ctrl_ext[1]: id=(10291459)V4L2_CID_JPEG_COMPRESSION_QUALITY type=V4L2_CTRL_TYPE_INTEGER name='Compression Quality' range=[1.+1.100] def=80 cur=80 flags=0000
```
