

NAME := App_WStudio_L8GameBleAws

$(NAME)_SOURCES := main.c \
    CapSenseThread.c \
	GameThread.c \
	cy_tft_display.c \
	GoBleThread.c \
	GoBle_db.c \
	wiced_bt_cfg.c \
	subscriber.c

$(NAME)_COMPONENTS := graphics/ugui \
                      libraries/drivers/bluetooth/low_energy \
                      protocols/AWS

WIFI_CONFIG_DCT_H := wifi_config_dct.h

$(NAME)_RESOURCES  := apps/aws/iot/rootca.cer \
                      apps/aws/iot/subscriber/client.cer \
                      apps/aws/iot/subscriber/privkey.cer
                      
# To support Low memory platforms, disabling components which are not required
GLOBAL_DEFINES += WICED_CONFIG_DISABLE_SSL_SERVER \
                  WICED_CONFIG_DISABLE_DTLS \
                  WICED_CONFIG_DISABLE_ENTERPRISE_SECURITY \
                  WICED_CONFIG_DISABLE_DES \
                  WICED_CONFIG_DISABLE_ADVANCED_SECURITY_CURVES