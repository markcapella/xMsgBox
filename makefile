
# *****************************************************
# Variables to control Compile / Link.
#
APP_NAME="xMsgBox"
APP_VERSION="2024-08-05"
APP_AUTHOR="Mark James Capella"

# Color styling.
COLOR_RED = \033[0;31m
COLOR_GREEN = \033[1;32m
COLOR_YELLOW = \033[1;33m
COLOR_BLUE = \033[1;34m
COLOR_NORMAL = \033[0m

CC = g++

X11_CFLAGS = `pkg-config --cflags x11`
FREETYPE_CFLAGS = `pkg-config --cflags xft`

X11_LFLAGS = `pkg-config --libs x11`
FREETYPE_LFLAGS = `pkg-config --libs xft`


# ****************************************************
# Compile & link.
#
all: xMsgBox.cpp
	@if [ "$(shell id -u)" = 0 ]; then \
		echo; \
		echo "$(COLOR_RED)Error!$(COLOR_NORMAL) You must not"\
			"be root to perform this action."; \
		echo; \
		echo  "Please re-run with:"; \
		echo "   $(COLOR_GREEN)make$(COLOR_NORMAL)"; \
		echo; \
		exit 1; \
	fi

	@echo
	@echo "$(COLOR_BLUE)Build Starts.$(COLOR_NORMAL)"
	@echo

	$(CC) $(X11_CFLAGS) $(FREETYPE_CFLAGS) \
		-o xMsgBox.o -c xMsgBox.cpp
	$(CC) xMsgBox.o $(X11_LFLAGS) $(FREETYPE_LFLAGS) \
		-o xMsgBox

	@echo
	@echo "$(COLOR_BLUE)Build Done.$(COLOR_NORMAL)"

	@echo "true" > "BUILD_COMPLETE"

# ****************************************************
# sudo make install
#
install:
	@if [ ! -f BUILD_COMPLETE ]; then \
		echo; \
		echo "$(COLOR_RED)Error!$(COLOR_NORMAL) Nothing"\
			"currently built to install."; \
		echo; \
		echo "Please make this project first, with:"; \
		echo "   $(COLOR_GREEN)make$(COLOR_NORMAL)"; \
		echo; \
		exit 1; \
	fi

	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo; \
		echo "$(COLOR_RED)Error!$(COLOR_NORMAL) You must"\
			"be root to perform this action."; \
		echo; \
		echo  "Please re-run with:"; \
		echo "   $(COLOR_GREEN)sudo make install$(COLOR_NORMAL)"; \
		echo; \
		exit 1; \
	fi

	@echo
	@echo "$(COLOR_BLUE)Install Starts.$(COLOR_NORMAL)"
	@echo

	cp 'xMsgBox' /usr/local/bin
	chmod +x /usr/local/bin/xMsgBox
	@echo

	cp 'xmsgboxinfo.png' /usr/share/icons/hicolor/48x48/apps/
	cp 'xmsgboxwarning.png' /usr/share/icons/hicolor/48x48/apps/
	cp 'xmsgboxerror.png' /usr/share/icons/hicolor/48x48/apps/

	@echo
	@echo "$(COLOR_BLUE)Install Done.$(COLOR_NORMAL)"

# ****************************************************
# sudo make uninstall
#
uninstall:
	@if ! [ "$(shell id -u)" = 0 ]; then \
		echo; \
		echo "$(COLOR_RED)Error!$(COLOR_NORMAL) You must"\
			"be root to perform this action."; \
		echo; \
		echo  "Please re-run with:"; \
		echo "   $(COLOR_GREEN)sudo make uninstall$(COLOR_NORMAL)"; \
		echo; \
		exit 1; \
	fi

	@echo
	@echo "$(COLOR_BLUE)Uninstall Starts.$(COLOR_NORMAL)"
	@echo

	rm -f /usr/local/bin/xMsgBox
	@echo

	rm -f /usr/share/icons/hicolor/48x48/apps/xmsgboxinfo.png
	rm -f /usr/share/icons/hicolor/48x48/apps/xmsgboxwarning.png
	rm -f /usr/share/icons/hicolor/48x48/apps/xmsgboxerror.png

	@echo
	@echo "$(COLOR_BLUE)Uninstall Done.$(COLOR_NORMAL)"

# ****************************************************
# make clean
#
clean:
	@if [ "$(shell id -u)" = 0 ]; then \
		echo; \
		echo "$(COLOR_RED)Error!$(COLOR_NORMAL) You must not"\
			"be root to perform this action."; \
		echo; \
		echo  "Please re-run with:"; \
		echo "   $(COLOR_GREEN)make clean$(COLOR_NORMAL)"; \
		echo; \
		exit 1; \
	fi

	@echo
	@echo "$(COLOR_BLUE)Clean Starts.$(COLOR_NORMAL)"
	@echo

	rm -f xMsgBox.o
	rm -f xMsgBox

	@rm -f "BUILD_COMPLETE"

	@echo
	@echo "$(COLOR_BLUE)Clean Done.$(COLOR_NORMAL)"
