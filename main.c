#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

static int state;

static Display *dpy;
static Window root;
static short screen;

static void
keypress(XEvent *ev) {
    XKeyEvent *kev;
    KeySym *keysym;
    int XXX;

    kev = &ev->xkey;

    keysym = XGetKeyboardMapping(dpy, kev->keycode, 1, &XXX);

    if (XK_Menu != *keysym)
        goto exit;

    state = 1;

exit:
    XFree(keysym);
}

static void
keyrelease(XEvent *ev) {
    XKeyEvent *kev;
    KeySym *keysym;
    int XXX;
    XEvent nev;

    kev = &ev->xkey;

    keysym = XGetKeyboardMapping(dpy, kev->keycode, 1, &XXX);

    if (XK_Menu != *keysym)
        goto exit;


    if (XEventsQueued(dpy, QueuedAfterReading))
    {
        XPeekEvent(dpy, &nev);

        if (KeyPress == nev.type &&
                nev.xkey.time == kev->time &&
                nev.xkey.keycode == kev->keycode)
            goto exit;
    }

    state = 0;

exit:
    XFree(keysym);
}

static int
x11_error(Display *dpy, XErrorEvent *ee) {
    warnx("Got an XErrorEvent: %p, %p\n", (void *)dpy, (void *)ee);

    return 0;
}

static void
x11_init(void) {
    if(NULL == (dpy = XOpenDisplay(NULL)))
        errx(EXIT_FAILURE, "Cannot open display");

    screen = DefaultScreen(dpy);
    root = RootWindow(dpy, screen);

    XSetErrorHandler(x11_error);

    XSelectInput(dpy, root, KeyPressMask);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XK_Menu), AnyModifier, root,
            False, GrabModeAsync, GrabModeAsync);
}

static void
mute(int do_mute)
{
    const char *card = "default";
    const char *selem_name = "Master";
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    snd_mixer_elem_t *elem;

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);

    elem = snd_mixer_find_selem(handle, sid);

    if (snd_mixer_selem_has_playback_switch(elem))
        snd_mixer_selem_set_playback_switch_all(elem, do_mute);

    snd_mixer_close(handle);
}

int
main(void) {
    static void(*handler[LASTEvent])(XEvent *) = {
        [KeyPress] = keypress,
        [KeyRelease] = keyrelease,
    };
    XEvent ev;
    int old_state;

    state = 0;

    x11_init();

    while(!XNextEvent(dpy, &ev))
    {
        if(!handler[ev.type])
            continue;

        old_state = state;

        handler[ev.type](&ev);

        if (state == old_state)
            continue;

        mute(!state);
    }

    return EXIT_SUCCESS;
}
