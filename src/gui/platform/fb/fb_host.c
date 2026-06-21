#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <termios.h>
#include <unistd.h>

typedef struct UyaGuiSimFbDisplay {
    int fd;
    uint8_t *mapped;
    size_t mapped_len;
    struct fb_fix_screeninfo finfo;
    struct fb_var_screeninfo vinfo;
    char device_path[256];
    int opened;
} UyaGuiSimFbDisplay;

typedef struct UyaGuiSimFbInput {
    int tty_fd;
    int evdev_fd;
    int opened;
    int esc_state;
    int termios_saved;
    int screen_w;
    int screen_h;
    int pointer_x;
    int pointer_y;
    int pointer_down;
    int abs_min_x;
    int abs_max_x;
    int abs_min_y;
    int abs_max_y;
    struct termios saved_termios;
    char tty_device_path[256];
    char evdev_device_path[256];
} UyaGuiSimFbInput;

typedef struct FbHostEvent {
    uint8_t kind;
    int16_t x;
    int16_t y;
    int32_t value;
    uint16_t key_code;
    uint16_t reserved;
} FbHostEvent;

enum {
    FB_EVT_NONE = 0,
    FB_EVT_KEY_DOWN = 1,
    FB_EVT_HOVER_MOVE = 2,
    FB_EVT_TOUCH_DOWN = 3,
    FB_EVT_TOUCH_UP = 4,
    FB_EVT_TOUCH_MOVE = 5,
    FB_EVT_WHEEL = 6,
};

static UyaGuiSimFbDisplay g_fb_display = {
    .fd = -1,
    .mapped = NULL,
    .mapped_len = 0u,
    .opened = 0,
};

static UyaGuiSimFbInput g_fb_input = {
    .tty_fd = -1,
    .evdev_fd = -1,
    .opened = 0,
    .esc_state = 0,
    .termios_saved = 0,
    .screen_w = 320,
    .screen_h = 240,
    .pointer_x = 160,
    .pointer_y = 120,
    .pointer_down = 0,
    .abs_min_x = 0,
    .abs_max_x = 319,
    .abs_min_y = 0,
    .abs_max_y = 239,
};

static char g_fb_last_error[256] = {0};
static char g_fb_input_last_error[256] = {0};

static void uya_gui_sim_fb_set_error(const char *message) {
    if (message == NULL || message[0] == '\0') {
        message = "unknown framebuffer error";
    }
    (void)snprintf(g_fb_last_error, sizeof(g_fb_last_error), "%s", message);
}

static void uya_gui_sim_fb_input_set_error(const char *message) {
    if (message == NULL || message[0] == '\0') {
        message = "unknown framebuffer input error";
    }
    (void)snprintf(g_fb_input_last_error, sizeof(g_fb_input_last_error), "%s", message);
}

static int uya_gui_sim_fb_clamp_i32(int value, int min_v, int max_v) {
    if (value < min_v) {
        return min_v;
    }
    if (value > max_v) {
        return max_v;
    }
    return value;
}

static int16_t uya_gui_sim_fb_map_abs(int raw, int raw_min, int raw_max, int screen_size) {
    int mapped;
    int clamped;
    if (screen_size <= 1 || raw_max <= raw_min) {
        return 0;
    }
    clamped = uya_gui_sim_fb_clamp_i32(raw, raw_min, raw_max);
    mapped = ((clamped - raw_min) * (screen_size - 1)) / (raw_max - raw_min);
    return (int16_t)uya_gui_sim_fb_clamp_i32(mapped, 0, screen_size - 1);
}

static void uya_gui_sim_fb_set_host_event(FbHostEvent *out_evt, uint8_t kind, int16_t x, int16_t y, int32_t value, uint16_t key_code) {
    out_evt->kind = kind;
    out_evt->x = x;
    out_evt->y = y;
    out_evt->value = value;
    out_evt->key_code = key_code;
    out_evt->reserved = 0u;
}

static int uya_gui_sim_fb_tty_keycode(uint8_t ch) {
    if (ch >= 32u && ch < 127u) {
        return (int)ch;
    }
    if (ch == 8u || ch == 127u) {
        return 8;
    }
    if (ch == '\r' || ch == '\n') {
        return 13;
    }
    if (ch == 3u) {
        return 27;
    }
    return 0;
}

static int uya_gui_sim_fb_linux_keycode(uint16_t code) {
    if (code >= KEY_A && code <= KEY_Z) {
        return 'A' + (int)(code - KEY_A);
    }
    if (code >= KEY_1 && code <= KEY_9) {
        return '1' + (int)(code - KEY_1);
    }
    if (code == KEY_0) {
        return '0';
    }
    switch (code) {
        case KEY_BACKSPACE: return 8;
        case KEY_ESC: return 27;
        case KEY_ENTER: return 13;
        case KEY_SPACE: return 32;
        case KEY_LEFT: return 1000;
        case KEY_RIGHT: return 1001;
        case KEY_UP: return 1002;
        case KEY_DOWN: return 1003;
        case KEY_F11: return 1011;
        default:
            return 0;
    }
}

static int uya_gui_sim_fb_tty_configure(int fd, struct termios *saved) {
    struct termios raw;
    if (tcgetattr(fd, saved) != 0) {
        return 0;
    }
    raw = *saved;
    raw.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    raw.c_oflag &= (tcflag_t)~(OPOST);
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    if (tcsetattr(fd, TCSANOW, &raw) != 0) {
        return 0;
    }
    return 1;
}

static uint32_t uya_gui_sim_scale_component(uint8_t value, uint32_t length) {
    if (length == 0u) {
        return 0u;
    }
    if (length >= 8u) {
        return ((uint32_t)value) << (length - 8u);
    }
    return (uint32_t)(value >> (8u - length));
}

static void uya_gui_sim_write_pixel(const UyaGuiSimFbDisplay *display, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    uint32_t pixel = 0u;
    const uint32_t bits_per_pixel = display->vinfo.bits_per_pixel;
    const uint32_t bytes_per_pixel = bits_per_pixel / 8u;
    const size_t offset = (size_t)y * (size_t)display->finfo.line_length + (size_t)x * bytes_per_pixel;
    if (bytes_per_pixel == 0u) {
        return;
    }
    if (offset + bytes_per_pixel > display->mapped_len) {
        return;
    }

    pixel |= uya_gui_sim_scale_component(r, display->vinfo.red.length) << display->vinfo.red.offset;
    pixel |= uya_gui_sim_scale_component(g, display->vinfo.green.length) << display->vinfo.green.offset;
    pixel |= uya_gui_sim_scale_component(b, display->vinfo.blue.length) << display->vinfo.blue.offset;
    if (display->vinfo.transp.length > 0u) {
        pixel |= uya_gui_sim_scale_component(a, display->vinfo.transp.length) << display->vinfo.transp.offset;
    }

    for (uint32_t i = 0; i < bytes_per_pixel; ++i) {
        display->mapped[offset + i] = (uint8_t)((pixel >> (i * 8u)) & 0xffu);
    }
}

void *uya_gui_sim_fb_open(const uint8_t *path) {
    UyaGuiSimFbDisplay *display = &g_fb_display;
    const char *env_path = getenv("UYA_GUI_FB_DEV");
    const char *dev_path = "/dev/fb0";
    if (display->opened) {
        return display;
    }
    if (path != NULL && path[0] != '\0') {
        dev_path = (const char *)path;
    } else if (env_path != NULL && env_path[0] != '\0') {
        dev_path = env_path;
    }

    memset(display, 0, sizeof(*display));
    display->fd = -1;
    (void)snprintf(display->device_path, sizeof(display->device_path), "%s", dev_path);

    display->fd = open(dev_path, O_RDWR);
    if (display->fd < 0) {
        uya_gui_sim_fb_set_error(strerror(errno));
        return NULL;
    }
    if (ioctl(display->fd, FBIOGET_FSCREENINFO, &display->finfo) != 0) {
        uya_gui_sim_fb_set_error(strerror(errno));
        (void)close(display->fd);
        display->fd = -1;
        return NULL;
    }
    if (ioctl(display->fd, FBIOGET_VSCREENINFO, &display->vinfo) != 0) {
        uya_gui_sim_fb_set_error(strerror(errno));
        (void)close(display->fd);
        display->fd = -1;
        return NULL;
    }

    display->mapped_len = (size_t)display->finfo.smem_len;
    display->mapped = mmap(NULL, display->mapped_len, PROT_READ | PROT_WRITE, MAP_SHARED, display->fd, 0);
    if (display->mapped == MAP_FAILED) {
        display->mapped = NULL;
        uya_gui_sim_fb_set_error(strerror(errno));
        (void)close(display->fd);
        display->fd = -1;
        return NULL;
    }

    display->opened = 1;
    g_fb_last_error[0] = '\0';
    return display;
}

void uya_gui_sim_fb_close(void *handle) {
    UyaGuiSimFbDisplay *display = (UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened) {
        return;
    }
    if (display->mapped != NULL) {
        (void)munmap(display->mapped, display->mapped_len);
    }
    if (display->fd >= 0) {
        (void)close(display->fd);
    }
    memset(display, 0, sizeof(*display));
    display->fd = -1;
}

int32_t uya_gui_sim_fb_present(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag) {
    UyaGuiSimFbDisplay *display = (UyaGuiSimFbDisplay *)handle;
    int copy_w;
    int copy_h;
    if (display == NULL || !display->opened || pixels == NULL) {
        uya_gui_sim_fb_set_error("framebuffer display not initialized");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_fb_set_error("framebuffer backend only supports ARGB8888 source");
        return 0;
    }

    copy_w = width < (int)display->vinfo.xres ? width : (int)display->vinfo.xres;
    copy_h = height < (int)display->vinfo.yres ? height : (int)display->vinfo.yres;
    for (int y = 0; y < copy_h; ++y) {
        const uint8_t *row = pixels + (size_t)y * (size_t)pitch;
        for (int x = 0; x < copy_w; ++x) {
            const uint8_t *src = row + (size_t)x * 4u;
            uya_gui_sim_write_pixel(display, x, y, src[1], src[2], src[3], src[0]);
        }
    }
    return 1;
}

int32_t uya_gui_sim_fb_present_region(void *handle, const uint8_t *pixels, int32_t pitch, int32_t width, int32_t height, int32_t format_tag,
                                      int32_t x0, int32_t y0, int32_t w, int32_t h) {
    UyaGuiSimFbDisplay *display = (UyaGuiSimFbDisplay *)handle;
    int max_w;
    int max_h;
    if (display == NULL || !display->opened || pixels == NULL) {
        uya_gui_sim_fb_set_error("framebuffer display not initialized");
        return 0;
    }
    if (format_tag != 2) {
        uya_gui_sim_fb_set_error("framebuffer backend only supports ARGB8888 source");
        return 0;
    }
    if (w <= 0 || h <= 0) {
        return 1;
    }
    if (x0 < 0 || y0 < 0 || x0 + w > width || y0 + h > height) {
        uya_gui_sim_fb_set_error("dirty rect out of bounds");
        return 0;
    }

    max_w = x0 + w;
    max_h = y0 + h;
    if (max_w > (int)display->vinfo.xres) {
        max_w = (int)display->vinfo.xres;
    }
    if (max_h > (int)display->vinfo.yres) {
        max_h = (int)display->vinfo.yres;
    }

    for (int y = y0; y < max_h; ++y) {
        const uint8_t *row = pixels + (size_t)y * (size_t)pitch;
        for (int x = x0; x < max_w; ++x) {
            const uint8_t *src = row + (size_t)x * 4u;
            uya_gui_sim_write_pixel(display, x, y, src[1], src[2], src[3], src[0]);
        }
    }
    return 1;
}

int32_t uya_gui_sim_fb_get_width(void *handle) {
    const UyaGuiSimFbDisplay *display = (const UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened) {
        return 0;
    }
    return (int32_t)display->vinfo.xres;
}

int32_t uya_gui_sim_fb_get_height(void *handle) {
    const UyaGuiSimFbDisplay *display = (const UyaGuiSimFbDisplay *)handle;
    if (display == NULL || !display->opened) {
        return 0;
    }
    return (int32_t)display->vinfo.yres;
}

const uint8_t *uya_gui_sim_fb_last_error(void) {
    return (const uint8_t *)g_fb_last_error;
}

static void uya_gui_sim_fb_try_open_evdev(UyaGuiSimFbInput *input, const char *path) {
    struct input_absinfo absinfo;
    input->evdev_fd = open(path, O_RDONLY | O_NONBLOCK);
    if (input->evdev_fd < 0) {
        uya_gui_sim_fb_input_set_error(strerror(errno));
        return;
    }
    if (ioctl(input->evdev_fd, EVIOCGABS(ABS_X), &absinfo) == 0 || ioctl(input->evdev_fd, EVIOCGABS(ABS_MT_POSITION_X), &absinfo) == 0) {
        input->abs_min_x = absinfo.minimum;
        input->abs_max_x = absinfo.maximum;
    }
    if (ioctl(input->evdev_fd, EVIOCGABS(ABS_Y), &absinfo) == 0 || ioctl(input->evdev_fd, EVIOCGABS(ABS_MT_POSITION_Y), &absinfo) == 0) {
        input->abs_min_y = absinfo.minimum;
        input->abs_max_y = absinfo.maximum;
    }
}

static void uya_gui_sim_fb_try_open_tty(UyaGuiSimFbInput *input, const char *path) {
    input->tty_fd = open(path, O_RDONLY | O_NONBLOCK);
    if (input->tty_fd < 0) {
        return;
    }
    if (!uya_gui_sim_fb_tty_configure(input->tty_fd, &input->saved_termios)) {
        (void)close(input->tty_fd);
        input->tty_fd = -1;
        return;
    }
    input->termios_saved = 1;
}

void *uya_gui_sim_fb_input_open(const uint8_t *input_path, const uint8_t *tty_path, int32_t width, int32_t height) {
    UyaGuiSimFbInput *input = &g_fb_input;
    const char *env_input_path = getenv("UYA_GUI_FB_INPUT");
    const char *env_tty_path = getenv("UYA_GUI_FB_TTY");
    const char *resolved_input_path = NULL;
    const char *resolved_tty_path = "/dev/tty";
    if (input->opened) {
        return input;
    }
    if (input_path != NULL && input_path[0] != '\0') {
        resolved_input_path = (const char *)input_path;
    } else if (env_input_path != NULL && env_input_path[0] != '\0') {
        resolved_input_path = env_input_path;
    }
    if (tty_path != NULL && tty_path[0] != '\0') {
        resolved_tty_path = (const char *)tty_path;
    } else if (env_tty_path != NULL && env_tty_path[0] != '\0') {
        resolved_tty_path = env_tty_path;
    }

    memset(input, 0, sizeof(*input));
    input->tty_fd = -1;
    input->evdev_fd = -1;
    input->screen_w = width > 0 ? width : 320;
    input->screen_h = height > 0 ? height : 240;
    input->pointer_x = input->screen_w / 2;
    input->pointer_y = input->screen_h / 2;
    input->abs_min_x = 0;
    input->abs_max_x = input->screen_w > 1 ? input->screen_w - 1 : 0;
    input->abs_min_y = 0;
    input->abs_max_y = input->screen_h > 1 ? input->screen_h - 1 : 0;
    (void)snprintf(input->tty_device_path, sizeof(input->tty_device_path), "%s", resolved_tty_path);
    if (resolved_input_path != NULL) {
        (void)snprintf(input->evdev_device_path, sizeof(input->evdev_device_path), "%s", resolved_input_path);
        uya_gui_sim_fb_try_open_evdev(input, resolved_input_path);
    }
    uya_gui_sim_fb_try_open_tty(input, resolved_tty_path);

    if (input->evdev_fd < 0 && input->tty_fd < 0) {
        if (g_fb_input_last_error[0] == '\0') {
            uya_gui_sim_fb_input_set_error("no framebuffer input source available");
        }
        return NULL;
    }

    input->opened = 1;
    g_fb_input_last_error[0] = '\0';
    return input;
}

void uya_gui_sim_fb_input_close(void *handle) {
    UyaGuiSimFbInput *input = (UyaGuiSimFbInput *)handle;
    if (input == NULL || !input->opened) {
        return;
    }
    if (input->termios_saved && input->tty_fd >= 0) {
        (void)tcsetattr(input->tty_fd, TCSANOW, &input->saved_termios);
    }
    if (input->tty_fd >= 0) {
        (void)close(input->tty_fd);
    }
    if (input->evdev_fd >= 0) {
        (void)close(input->evdev_fd);
    }
    memset(input, 0, sizeof(*input));
    input->tty_fd = -1;
    input->evdev_fd = -1;
}

static int32_t uya_gui_sim_fb_input_poll_evdev(UyaGuiSimFbInput *input, FbHostEvent *out_evt) {
    if (input == NULL || input->evdev_fd < 0) {
        return 0;
    }

    while (1) {
        struct input_event ev;
        const ssize_t n = read(input->evdev_fd, &ev, sizeof(ev));
        if (n == 0 || (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
            return 0;
        }
        if (n < 0) {
            uya_gui_sim_fb_input_set_error(strerror(errno));
            return 0;
        }
        if ((size_t)n != sizeof(ev)) {
            continue;
        }

        if (ev.type == EV_KEY) {
            if (ev.code == BTN_TOUCH || ev.code == BTN_LEFT) {
                input->pointer_down = ev.value != 0;
                uya_gui_sim_fb_set_host_event(
                    out_evt,
                    ev.value ? FB_EVT_TOUCH_DOWN : FB_EVT_TOUCH_UP,
                    (int16_t)input->pointer_x,
                    (int16_t)input->pointer_y,
                    0,
                    0u
                );
                return 1;
            }
            if (ev.value == 1) {
                const int keycode = uya_gui_sim_fb_linux_keycode(ev.code);
                if (keycode != 0) {
                    uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, (uint16_t)keycode);
                    return 1;
                }
            }
            continue;
        }

        if (ev.type == EV_REL) {
            if (ev.code == REL_X) {
                input->pointer_x = uya_gui_sim_fb_clamp_i32(input->pointer_x + (int)ev.value, 0, input->screen_w - 1);
                uya_gui_sim_fb_set_host_event(
                    out_evt,
                    input->pointer_down ? FB_EVT_TOUCH_MOVE : FB_EVT_HOVER_MOVE,
                    (int16_t)input->pointer_x,
                    (int16_t)input->pointer_y,
                    0,
                    0u
                );
                return 1;
            }
            if (ev.code == REL_Y) {
                input->pointer_y = uya_gui_sim_fb_clamp_i32(input->pointer_y + (int)ev.value, 0, input->screen_h - 1);
                uya_gui_sim_fb_set_host_event(
                    out_evt,
                    input->pointer_down ? FB_EVT_TOUCH_MOVE : FB_EVT_HOVER_MOVE,
                    (int16_t)input->pointer_x,
                    (int16_t)input->pointer_y,
                    0,
                    0u
                );
                return 1;
            }
            if (ev.code == REL_WHEEL) {
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_WHEEL, 0, 0, (int32_t)ev.value, 0u);
                return 1;
            }
            continue;
        }

        if (ev.type == EV_ABS) {
            if (ev.code == ABS_X || ev.code == ABS_MT_POSITION_X) {
                input->pointer_x = uya_gui_sim_fb_map_abs((int)ev.value, input->abs_min_x, input->abs_max_x, input->screen_w);
                uya_gui_sim_fb_set_host_event(
                    out_evt,
                    input->pointer_down ? FB_EVT_TOUCH_MOVE : FB_EVT_HOVER_MOVE,
                    (int16_t)input->pointer_x,
                    (int16_t)input->pointer_y,
                    0,
                    0u
                );
                return 1;
            }
            if (ev.code == ABS_Y || ev.code == ABS_MT_POSITION_Y) {
                input->pointer_y = uya_gui_sim_fb_map_abs((int)ev.value, input->abs_min_y, input->abs_max_y, input->screen_h);
                uya_gui_sim_fb_set_host_event(
                    out_evt,
                    input->pointer_down ? FB_EVT_TOUCH_MOVE : FB_EVT_HOVER_MOVE,
                    (int16_t)input->pointer_x,
                    (int16_t)input->pointer_y,
                    0,
                    0u
                );
                return 1;
            }
            if (ev.code == ABS_WHEEL) {
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_WHEEL, 0, 0, (int32_t)ev.value, 0u);
                return 1;
            }
            continue;
        }
    }
}

static int32_t uya_gui_sim_fb_input_poll_tty(UyaGuiSimFbInput *input, FbHostEvent *out_evt) {
    if (input == NULL || input->tty_fd < 0) {
        return 0;
    }

    while (1) {
        uint8_t ch = 0u;
        const ssize_t n = read(input->tty_fd, &ch, 1u);
        if (n == 0 || (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))) {
            if (input->esc_state == 1) {
                input->esc_state = 0;
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, 27u);
                return 1;
            }
            return 0;
        }
        if (n < 0) {
            uya_gui_sim_fb_input_set_error(strerror(errno));
            return 0;
        }

        if (input->esc_state == 0) {
            if (ch == 0x1bu) {
                input->esc_state = 1;
                continue;
            }
            const int keycode = uya_gui_sim_fb_tty_keycode(ch);
            if (keycode != 0) {
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, (uint16_t)keycode);
                return 1;
            }
            continue;
        }

        if (input->esc_state == 1) {
            if (ch == '[') {
                input->esc_state = 2;
                continue;
            }
            input->esc_state = 0;
            uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, 27u);
            return 1;
        }

        input->esc_state = 0;
        switch (ch) {
            case 'A':
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, 1002u);
                return 1;
            case 'B':
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, 1003u);
                return 1;
            case 'C':
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, 1001u);
                return 1;
            case 'D':
                uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_KEY_DOWN, 0, 0, 0, 1000u);
                return 1;
            default:
                break;
        }
    }
}

int32_t uya_gui_sim_fb_input_poll_event(void *handle, FbHostEvent *out_evt) {
    UyaGuiSimFbInput *input = (UyaGuiSimFbInput *)handle;
    if (out_evt == NULL) {
        return 0;
    }
    uya_gui_sim_fb_set_host_event(out_evt, FB_EVT_NONE, 0, 0, 0, 0u);
    if (input == NULL || !input->opened) {
        return 0;
    }
    if (uya_gui_sim_fb_input_poll_evdev(input, out_evt) != 0) {
        return 1;
    }
    return uya_gui_sim_fb_input_poll_tty(input, out_evt);
}

const uint8_t *uya_gui_sim_fb_input_last_error(void) {
    return (const uint8_t *)g_fb_input_last_error;
}
