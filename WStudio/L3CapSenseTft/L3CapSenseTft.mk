
NAME := App_WStudio_L3CapSenseTft

$(NAME)_SOURCES := 	main.c \
				CapSenseThread.c \
				GameThread.c \
				cy_tft_display.c
				
				

$(NAME)_COMPONENTS := graphics/ugui
			