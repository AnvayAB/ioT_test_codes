#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#define BUFFER_COUNT 4

struct buffer {
    void *start;
    size_t length;
};

static struct buffer* buffers;
static int fd = -1;

void init_device(const char *dev_name) {
    struct v4l2_capability cap;
    struct v4l2_format fmt;
    struct v4l2_requestbuffers req;
    struct v4l2_buffer buf;
    unsigned int i;

    fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);
    if (fd == -1) {
        perror("Error opening device");
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) == -1) {
        perror("Error querying device capabilities");
        exit(EXIT_FAILURE);
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "Device does not support video capture\n");
        exit(EXIT_FAILURE);
    }

    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1) {
        perror("Error getting pixel format");
        exit(EXIT_FAILURE);
    }

    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Error setting pixel format");
        exit(EXIT_FAILURE);
    }

    memset(&req, 0, sizeof(req));
    req.count = BUFFER_COUNT;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Error requesting buffers");
        exit(EXIT_FAILURE);
    }

    buffers = calloc(req.count, sizeof(*buffers));
    if (!buffers) {
        perror("Error allocating memory for buffers");
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < req.count; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
            perror("Error querying buffer");
            exit(EXIT_FAILURE);
        }

        buffers[i].length = buf.length;
        buffers[i].start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

        if (buffers[i].start == MAP_FAILED) {
            perror("Error mapping buffer");
            exit(EXIT_FAILURE);
        }
    }
}

void capture_image() {
    struct v4l2_buffer buf;
    unsigned int i;

    for (i = 0; i < BUFFER_COUNT; ++i) {
        memset(&buf, 0, sizeof(buf));
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
            perror("Error queuing buffer");
            exit(EXIT_FAILURE);
        }
    }

    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1) {
        perror("Error starting stream");
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Error dequeuing buffer");
        exit(EXIT_FAILURE);
    }

    // Process the captured frame stored in buffers[buf.index].start
    // ...

    if (ioctl(fd, VIDIOC_STREAMOFF, &type) == -1) {
        perror("Error stopping stream");
        exit(EXIT_FAILURE);
    }
}

void cleanup_device() {
    unsigned int i;
    for (i = 0; i < BUFFER_COUNT; ++i) {
        if (munmap(buffers[i].start, buffers[i].length) == -1) {
            perror("Error unmapping buffer");
        }
    }
    free(buffers);

    if (fd != -1 && close(fd) == -1) {
        perror("Error closing device");
    }
}

int main() {
    const char *device_name = "/dev/video0";  // Change to the correct device path

    init_device(device_name);
    capture_image();
    cleanup_device();

    return 0;
}