

NAME := App_WStudio_GameBle

$(NAME)_SOURCES := main.c \
	GameThread.c \
	cy_tft_display.c \
	GoBleThread.c \
	GoBle_db.c \
	wiced_bt_cfg.c

$(NAME)_COMPONENTS := graphics/ugui \
                      libraries/drivers/bluetooth/low_energy
