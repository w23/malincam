- [x] Get bayer images from sensor
- [ ] Buffer carousel: on each pad/endpoint have an outstanding buffer. It gets replaced any time a new buffer is
      dequeued. Similar with inverse direction: buffers returning from outside are written into a queue.
      - [ ] Q: how should it signal buffer availability in both ways, if the other side is idle and waiting?-
- [ ] Buffer pumper: pump buffers between endpoints on select/poll/epoll.
      This is the thing that should poke carousels above. How?
      It should also be aware of buffer being in use (esp. for DMABUF stuff).
      Threading? E.g. writing into a file/network/etc
- [ ] Use /dev/video12 to convert bayer to YUV or something mjpeg supports
    - is it the right one?
- [ ] Use /dev/video31 (??) to encode MJPEG.
- [ ] Make UVC gadget device to send all this crap into.
    - [ ] set it up
    - [ ] open v4l
    - [ ] feed it
