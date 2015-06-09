TEMPLATE = subdirs

SUBDIRS += \
    #audioout \
    rtp \
    rtsp

unix:!macx {
    SUBDIRS += \
    devicecontrol
}
