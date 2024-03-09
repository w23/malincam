- [x] Get bayer images from sensor
- [x] print out the format that has been set
- [-] name devices and streams
- [x] Buffer carousel: on each pad/endpoint have an outstanding buffer. It gets replaced any time a new buffer is
      dequeued. Similar with inverse direction: buffers returning from outside are written into a queue.
      - [x] Q: how should it signal buffer availability in both ways, if the other side is idle and waiting?
            A: seems to just work -- poll() only wakes up whenever something does change and can be polled.
- [x] Buffer pumper: pump buffers between endpoints on select/poll/epoll.
      This is the thing that should poke carousels above. How?
      It should also be aware of buffer being in use (esp. for DMABUF stuff).
      Threading? E.g. writing into a file/network/etc
- [x] :x: Use /dev/video12 to convert bayer to YUV or something mjpeg supports
    - is it the right one? -- this is a simple ISP, it lacks various image controls.
    - It also doesn't work for some reason.
        → decided not to use
- [x] Use /dev/video31 (??) to encode MJPEG.
- [x] Make UVC gadget device to send all this crap into.
    - [x] set it up
    - [x] open v4l
    - [x] feed it
- [ ] configurable debug messages
- [ ] graceful stream on/off
- [ ] controls
- [ ] sensor flipping and format
- [ ] expand to h=992 instead of cropping to 976
      - possibly set isp to 992, and then crop image_encode to 990
- [ ] crop from center
- [ ] resolution selector
- [ ] framerate selector
- [ ] format selector
- [ ] more camera types
