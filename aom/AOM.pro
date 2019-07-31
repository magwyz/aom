# Created by and for Qt Creator This file was created for editing the project sources only.
# You may attempt to use it for building too, by modifying this file here.

#TARGET = AOM

HEADERS = \
   $$PWD/internal/aom_codec_internal.h \
   $$PWD/internal/aom_image_internal.h \
   $$PWD/aom.h \
   $$PWD/aom_codec.h \
   $$PWD/aom_decoder.h \
   $$PWD/aom_encoder.h \
   $$PWD/aom_frame_buffer.h \
   $$PWD/aom_image.h \
   $$PWD/aom_integer.h \
   $$PWD/aomcx.h \
   $$PWD/aomdx.h

SOURCES = \
   $$PWD/src/aom_codec.c \
   $$PWD/src/aom_decoder.c \
   $$PWD/src/aom_encoder.c \
   $$PWD/src/aom_image.c \
   $$PWD/src/aom_integer.c

INCLUDEPATH = \
    $$PWD/. \
    $$PWD/internal

#DEFINES = 

