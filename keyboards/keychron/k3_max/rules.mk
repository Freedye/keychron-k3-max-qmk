include keyboards/keychron/common/wireless/wireless.mk
include keyboards/keychron/common/keychron_common.mk

VPATH += $(TOP_DIR)/keyboards/keychron

# Keychron ChibiOS fork patches the K3 Max needs, applied on top of QMK's ChibiOS
SRC += chibios_fixes.c
