AM_CFLAGS = -Wall -Wextra -pedantic -std=c99

bin_PROGRAMS = mute

mute_CFLAGS = $(X11_CFLAGS) $(ALSA_CFLAGS)
mute_LDADD = $(X11_LIBS) $(ALSA_LIBS)
mute_SOURCES = main.c 
