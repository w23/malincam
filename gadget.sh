#!/bin/bash
set -eux

VID="0x0525"
PID="0xa4a2"
SERIAL="0123456789"
MANUFACTURER="Malincam"
PRODUCT="Malincam"

# These variables will be assumed throughout the rest of the document
CONFIGFS="/sys/kernel/config"
GADGET="$CONFIGFS/usb_gadget/g1"
FUNCTION="$GADGET/functions/uvc.0"
UDC=$(ls /sys/class/udc) # will identify the 'first' UDC

uvc_setup_basics() {
	modprobe libcomposite

	mkdir -p "$GADGET"

	pushd "$GADGET"

	echo $VID > idVendor
	echo $PID > idProduct

	mkdir strings/0x409
	echo $SERIAL > strings/0x409/serialnumber
	echo $MANUFACTURER > strings/0x409/manufacturer
	echo $PRODUCT > strings/0x409/product

	popd
}

uvc_create_frame() {
	# Example usage:
	# create_frame <width> <height> <group> <format name>

	WIDTH=$1
	HEIGHT=$2
	FORMAT=$3
	NAME=$4

	# TODO framerate as the rest of args

	wdir="$FUNCTION/streaming/$FORMAT/$NAME/${HEIGHT}p"

	mkdir -p $wdir
	echo $WIDTH > $wdir/wWidth
	echo $HEIGHT > $wdir/wHeight
	echo $(( $WIDTH * $HEIGHT * 2 )) > $wdir/dwMaxVideoFrameBufferSize

	# in units of 100ns, truncated (not rounded) to int
	# 120 fps = 83333
	# 60 fps = 166666
	# 50 fps = 200000
	# 30 fps = 333333
	# 15 fps = 666666
	cat <<EOF > $wdir/dwFrameInterval
83333
EOF
}

uvc_setup_modes() {
	uvc_create_frame 1332 976 mjpeg mjpeg
	#create_frame 1920 1080 mjpeg mjpeg
	#create_frame 1280 720 uncompressed yuyv
	#create_frame 1920 1080 uncompressed yuyv
}

uvc_setup_colorspaces() {
	# TODO

	# Create a new Color Matching Descriptor
	mkdir $FUNCTION/streaming/color_matching/yuyv
	pushd $FUNCTION/streaming/color_matching/yuyv

	echo 1 > bColorPrimaries
	echo 1 > bTransferCharacteristics
	echo 4 > bMatrixCoefficients

	popd

	# Create a symlink to the Color Matching Descriptor from the format's config item
	ln -s $FUNCTION/streaming/color_matching/yuyv $FUNCTION/streaming/uncompressed/yuyv
}

# ???? 
uvc_link_headers() {
	mkdir $FUNCTION/streaming/header/h

	# This section links the format descriptors and their associated frames
	# to the header
	pushd $FUNCTION/streaming/header/h

	#TODO ln -s ../../uncompressed/yuyv
	ln -s ../../mjpeg/mjpeg

	# This section ensures that the header will be transmitted for each
	# speed's set of descriptors. If support for a particular speed is not
	# needed then it can be skipped here.
	cd ../../class/fs
	ln -s ../../header/h
	cd ../../class/hs
	ln -s ../../header/h
	cd ../../class/ss
	ln -s ../../header/h
	cd ../../../control
	mkdir header/h
	ln -s header/h class/fs
	ln -s header/h class/ss

	popd
}

# Set up regular (non-XU) UVC controls
uvc_setup_controls() {
	# TODO specify real controls that we have

	# Set the Processing Unit's bmControls, flagging Brightness, Contrast
	# and Hue as available controls:
	echo 0x05 > $FUNCTION/control/processing/default/bmControls

	# Set the Camera Terminal's bmControls, flagging Focus Absolute and
	# Focus Relative as available controls:
	echo 0x60 > $FUNCTION/control/terminal/camera/default/bmControls
}

uvc_setup_bandwidth() {
	# streaming_interval sets bInterval. Values range from 1..255
	echo 1 > $FUNCTION/streaming_interval

	# streaming_maxpacket sets wMaxPacketSize. Valid values are 1024/2048/3072
	# Need all three, otherwise `dwc2 3f980000.usb: dwc2_hsotg_ep_enable: No suitable fifo found` will happen
	echo 3072 > $FUNCTION/streaming_maxpacket
	echo 2048 > $FUNCTION/streaming_maxpacket
	echo 1024 > $FUNCTION/streaming_maxpacket

	# streaming_maxburst sets bMaxBurst. Valid values are 1..15
	echo 1 > $FUNCTION/streaming_maxburst
}

uvc_make_function() {
	mkdir -p "$FUNCTION"

	uvc_setup_modes
	#uvc_setup_colorspaces

	#TODO uvc_setup_controls

	uvc_link_headers
	uvc_setup_bandwidth
}

uvc_make_config() {
	# TODO ???
	echo "Creating Config"
	mkdir "$GADGET"/configs/c.1
	mkdir "$GADGET"/configs/c.1/strings/0x409
}

uvc_create() {
	uvc_setup_basics

	uvc_make_config
	uvc_make_function

	ln -s "$FUNCTION" "$GADGET"/configs/c.1

	echo "$UDC" > $GADGET/UDC
}

uvc_destroy() {
	echo "" > $GADGET/UDC || echo "$?"

	rm $GADGET/configs/c.1/uvc.0 || echo "$?"
	rm $GADGET/configs/c.1/strings/0x409/* || echo "$?"
	rmdir $GADGET/configs/c.1/strings/0x409 || echo "$?"
	rm $GADGET/configs/c.1 || echo "$?"

	rm $FUNCTION/control/class/*/h || echo "$?"
	rm $FUNCTION/streaming/class/*/h || echo "$?"
	rm $FUNCTION/streaming/header/h/u || echo "$?"
	#rmdir $FUNCTION/streaming/uncompressed/yuyv/*/ || echo "$?"
	#rmdir $FUNCTION/streaming/uncompressed/yuyv || echo "$?"
	rmdir $FUNCTION/streaming/mjpeg/mjpeg/*p || echo "$?"
	#rm -rf $FUNCTION/streaming/mjpeg/mjpeg/*/ || echo "$?"
	rmdir $FUNCTION/streaming/mjpeg/mjpeg || echo "$?"
	rmdir $FUNCTION/streaming/mjpeg || echo "$?"
	#rm -rf $FUNCTION/streaming/mjpeg/mjpeg || echo "$?"
	rmdir $FUNCTION/streaming/header/h || echo "$?"
	rmdir $FUNCTION/control/header/h || echo "$?"
	rmdir $FUNCTION || echo "$?"

	rmdir $GADGET/strings/0x409 || echo "$?"

	rmdir $GADGET
}

"$@"
