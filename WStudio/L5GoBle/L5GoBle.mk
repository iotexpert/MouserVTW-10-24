
NAME := App_WStudio_L5GoBle

$(NAME)_SOURCES    := GoBle.c \
			          GoBleThread.c \
                      GoBle_db.c \
                      wiced_bt_cfg.c

$(NAME)_INCLUDES   := .

$(NAME)_COMPONENTS += libraries/drivers/bluetooth/low_energy 
