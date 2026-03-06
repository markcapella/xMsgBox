
/**
 * This is a multi-line GUI Message Box for bash
 * scripts. Built with native X11 & Xf.
 */

// Std C and c++.
#include <iostream>
#include <list>
#include <string>
#include <string.h>

using namespace std;

// X11.
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// Application.
#include "xMsgBox.h"
#include "xDisplayHelper.h"
#include "xPngWrapper.h"


/**
 * Module Consts.
 */
const string ICON_PNGPATH =
    "/usr/share/icons/hicolor/48x48/apps/";

const string INFO_PNGFILE = "xmsgboxinfo.png";
const string WARNING_PNGFILE = "xmsgboxwarning.png";
const string ERROR_PNGFILE = "xmsgboxerror.png";

const XftColor mFontColor = {
    .pixel = 0x0, .color = {
        .red = 0xff, .green = 0xff,
        .blue = 0xff, .alpha = 0xffff
    }
};

/**
 * Module globals.
 */
xDisplayHelper* mDisplayHelper;
Display* mDisplay;

Window mMsgBox;
XftFont* mFont;
xPngWrapper* mIconWrapper;

int mMsgBoxXPos;
int mMsgBoxYPos;

string mMsgBoxTitle;

int mMsgBoxWindowWidth;
int mMsgBoxWindowHeight;

int mMsgBoxTextareaWidth;
int mMsgBoxTextareaHeight;

std::list<string> mMsgBoxLines;

/**
 * Module Entry.
 */
int main(int argCount, char** argValues) {
    // Ensure proper invocation.
    if (argCount < APP_PARMS_REQUIRED) {
        displayUsage();
        return true;
    }

    // Parse invocation.
    mMsgBoxXPos = atoi(argValues[1]);
    mMsgBoxYPos = atoi(argValues[2]);
    mMsgBoxTitle += argValues[3];

    // Get wrapped PNG file for the Icon.
    char* pngFileName = mMsgBoxTitle == "Error" ?
        strdup(ERROR_PNGFILE.c_str()) :
        mMsgBoxTitle == "Warning" ?
            strdup(WARNING_PNGFILE.c_str()) :
            strdup(INFO_PNGFILE.c_str());
    mIconWrapper = new xPngWrapper({
        string(pngFileName),
        string(ICON_PNGPATH + pngFileName)
    });

    if (mIconWrapper->hasErrorStatus()) {
        cout << COLOR_RED << endl << "xMsgBox: " <<
            mIconWrapper->errorStatus() <<
            COLOR_NORMAL << endl;
        return true;
    }

    // Open X11 display, ensure it's available.
    mDisplayHelper = new xDisplayHelper();
    mDisplay = mDisplayHelper->getDisplay();
    if (mDisplay == NULL) {
        cout << COLOR_RED << "\nxMsgBox: X11 Windows are "
            "unavailable with this desktop. - FATAL" <<
            COLOR_NORMAL << "\n";
        exit(2);
    }

    // Set font for layouts.
    mFont = XftFontOpenName(mDisplay,
        DefaultScreen(mDisplay), "");
    if (mFont == NULL) {
        cout << COLOR_RED << "\nxMsgBox: Cannot open XftFont - "
            "FATAL.\n" << COLOR_NORMAL;
        exit(3);
    }

    // Dertermine largest message string width & height;
    mMsgBoxTextareaWidth = getMaxMsgPixelWidth(argCount, argValues);
    mMsgBoxTextareaHeight = getMaxMsgPixelHeight(argCount, argValues);

    // Dertermine overall window size.
    mMsgBoxWindowWidth = LEFT_MARGIN + mMsgBoxTextareaWidth + RIGHT_MARGIN;
    mMsgBoxWindowHeight = TOP_MARGIN + getYPosForMessageBoxLineIndex(
        getNumberOfMessages(argCount) - 1, mMsgBoxTextareaHeight,
        PIXELS_BETWEEN_MSGBOX_TEXTAREA_LINES) + BOTTOM_MARGIN;

    // Log debug info for all layout values.
    if (DEBUG_IS_ON) {
        logDebugValues(argCount);
    }

    // Create xMsgBox from X11 window.
    mMsgBox = XCreateSimpleWindow(mDisplay,
        DefaultRootWindow(mDisplay), 0, 0,
        mMsgBoxWindowWidth, mMsgBoxWindowHeight, 1,
        BlackPixel(mDisplay, 0), WhitePixel(mDisplay, 0));

    // Set title string.
    XTextProperty properties;
    properties.value = (unsigned char*) mMsgBoxTitle.c_str();
    properties.encoding = XA_STRING;
    properties.format = 8;
    properties.nitems = mMsgBoxTitle.length();
    XSetWMName(mDisplay, mMsgBox, &properties);

    // Set icon name strings.
    XClassHint* classHint = XAllocClassHint();
    if (classHint) {
        classHint->res_class = pngFileName;
        classHint->res_name = pngFileName;
        XSetClassHint(mDisplay, mMsgBox, classHint);
        XFree(classHint);
    }
    XTextProperty iconProperty;
    XStringListToTextProperty(&pngFileName, 1,
        &iconProperty);
    XSetWMIconName(mDisplay, mMsgBox, &iconProperty);

    // Set the _NET_WM_ICON property from the vector.
    const Atom net_wm_icon = XInternAtom(mDisplay,
        "_NET_WM_ICON", False);
    XChangeProperty(mDisplay, mMsgBox, net_wm_icon,
        XA_CARDINAL, 32, PropModeReplace,
        reinterpret_cast<unsigned char*>
            (mIconWrapper->getPngData().data()),
             mIconWrapper->getPngData().size());

    // Map (show) xMsgBox window.
    XMapWindow(mDisplay, mMsgBox);
    XMoveWindow(mDisplay, mMsgBox, mMsgBoxXPos, mMsgBoxYPos);

    // Select observable x11 events &
    // Select observable x11 client messages.
    XSelectInput(mDisplay, mMsgBox, ExposureMask);
    Atom mDeleteMessage = XInternAtom(mDisplay,
        "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, mMsgBox, &mDeleteMessage, 1);

    // Loop until close event frees us.
    bool msgboxActive = true;
    while (msgboxActive) {
        XEvent event;
        XNextEvent(mDisplay, &event);

        // Process ClientMsg Close event.
        if (event.type == ClientMessage) {
            if (event.xclient.data.l[0] == mDeleteMessage) {
                msgboxActive = false;
            }
            break;
        }

        // Process Expose event. Set the xMsgBox
        // Msg on Window expose.
        if (event.type == Expose) {
            if (XPending(mDisplay) == 0) {
                const XExposeEvent* EVENT =
                    (XExposeEvent*) &event;
                if (EVENT->width > 1 && EVENT->height > 1) {
                    drawMessageBox(argCount, argValues);
                    if (DEBUG_IS_ON) {
                        drawMessageBoxOutLines(argCount);
                    }
                }
            }
        }
    }

    // Close display & done.
    XUnmapWindow(mDisplay, mMsgBox);
    XDestroyWindow(mDisplay, mMsgBox);
    XCloseDisplay(mDisplay);
}

/**
 * This method displays the basic use syntax.
 */
void displayUsage() {
    cout << COLOR_BLUE << "\nUsage:" << COLOR_NORMAL << "\n";
    cout << COLOR_GREEN << "   xMsgBox xPos yPos "
        "title message message2 message3 ..." <<
        COLOR_NORMAL << "\n";

    cout << COLOR_BLUE << "\nExample:" << COLOR_NORMAL << "\n";
    cout << COLOR_GREEN << "   xMsgBox 600 400 \"Error\" "
        "\"Something failed.\" \"Please try later.\"" <<
        COLOR_NORMAL << "\n";
}

/**
 * This method returns pixel witdh of a text string.
 */
int getMaxMsgPixelWidth(int argCount, char** argValues) {
    int index = getFirstMessageArgI();
    const int indexEnd = getLastMessageArgI(argCount);

    int resultWidth = 0;
    while (index <= indexEnd) {
        const int width = getStringPixelWidth(argValues[index]);
        if (width > resultWidth) {
            resultWidth = width;
        }
        index++;
    }
    return resultWidth;
}

/**
 * This method returns pixel height of a textSring.
 */
int getMaxMsgPixelHeight(int argCount, char** argValues) {
    int index = getFirstMessageArgI();
    const int indexEnd = getLastMessageArgI(argCount);

    int resultHeight = 0;
    while (index <= indexEnd) {
        const int height = getStringPixelHeight(argValues[index]);
        if (height > resultHeight) {
            resultHeight = height;
        }
        index++;
    }
    return resultHeight;
}

/**
 * This method returns pixel witdh of a text string.
 */
int getStringPixelWidth(string textString) {
    XGlyphInfo textExtents;
    XftTextExtents8(mDisplay, mFont, (const FcChar8*)
        textString.c_str(), textString.length(), &textExtents);

    return textExtents.width;
}

/**
 * This method returns pixel height of a text string.
 */
int getStringPixelHeight(string textString) {
    XGlyphInfo textExtents;
    XftTextExtents8(mDisplay, mFont, (const FcChar8*)
        textString.c_str(), textString.length(), &textExtents);

    return textExtents.height;
}

/**
 * This method draws an visual outline of the xMsgBox
 * for debugging. S/b safe to remove / deprecate.
 */
void drawMessageBoxOutLines(int argCount) {
    GC gc = XCreateGC(mDisplay, mMsgBox, 0, NULL);

    // Slash line from top-left to bottom-right of entire Window.
    XDrawLine(mDisplay, mMsgBox, gc,
        0, 0, mMsgBoxWindowWidth, mMsgBoxWindowHeight);

    // Draw top, bottom, left & right borders
    // around Messages textarea.
    XDrawLine(mDisplay, mMsgBox, gc, LEFT_MARGIN, TOP_MARGIN,
        LEFT_MARGIN + mMsgBoxTextareaWidth, TOP_MARGIN);

    XDrawLine(mDisplay, mMsgBox, gc, LEFT_MARGIN,
        mMsgBoxWindowHeight - BOTTOM_MARGIN, LEFT_MARGIN +
        mMsgBoxTextareaWidth, mMsgBoxWindowHeight - BOTTOM_MARGIN);

    XDrawLine(mDisplay, mMsgBox, gc, LEFT_MARGIN, TOP_MARGIN,
        LEFT_MARGIN, mMsgBoxWindowHeight - BOTTOM_MARGIN);

    XDrawLine(mDisplay, mMsgBox, gc, LEFT_MARGIN +
        mMsgBoxTextareaWidth, TOP_MARGIN, LEFT_MARGIN +
        mMsgBoxTextareaWidth, mMsgBoxWindowHeight - BOTTOM_MARGIN);
}

/**
 * Draw all message strings centered in the window.
 */
void drawMessageBox(int argCount, char** argValues) {
    int index = getFirstMessageArgI();
    const int indexEnd = getLastMessageArgI(argCount);

    int lineIndex = 0;
    while (index <= indexEnd) {
        XftDraw* textDrawable = XftDrawCreate(mDisplay, mMsgBox,
            DefaultVisual(mDisplay, DefaultScreen(mDisplay)),
           DefaultColormap(mDisplay, DefaultScreen(mDisplay)));

        const int lineYPos = TOP_MARGIN + getYPosForMessageBoxLineIndex(
            lineIndex, mMsgBoxTextareaHeight, PIXELS_BETWEEN_MSGBOX_TEXTAREA_LINES);

        XftDrawString8(textDrawable, &mFontColor, mFont,
            LEFT_MARGIN, lineYPos, (const FcChar8*) argValues[index],
            strlen(argValues[index]));

        lineIndex++; index++;
    }
}

/**
 * This method determines pixel offset for a message
 * string drawline.
 */
long getYPosForMessageBoxLineIndex(long lineIndex,
    long lineHeight, long lineSpace) {

    return ((lineIndex + 1) * (lineHeight + lineSpace));
}

/**
 * This method returns the number of message strings that
 * xMsgBox is to display from the users cmdline.
 */
int getNumberOfMessages(int argCount) {
    return getLastMessageArgI(argCount) -
        getFirstMessageArgI() + 1;
}

/**
 * This method returns the argi of argv[] of the First
 * message string that xMsgBox is to display from
 * the users cmdline.
 */
int getFirstMessageArgI() {
    return APP_PARMS_REQUIRED - 1;
}

/**
 * This method returns the argi of argv[] of the Last
 * message string that xMsgBox is to display from
 * the users cmdline.
 */
int getLastMessageArgI(int argCount) {
    return argCount - 1;
}

/**
 * This method logs critical info for devs.
 */
void logDebugValues(int argCount) {
    cout << "argCount                : [" << argCount << "]\n";
    cout << "APP_PARMS_REQUIRED      : [" <<
        getNumberOfMessages(argCount) << "]\n";
    cout << "\n";

    cout << "getNumberOfMessages      : [" <<
        getNumberOfMessages(argCount) << "]\n";
    cout << "getFirstMessageArgI : [" <<
        getFirstMessageArgI() << "]\n";
    cout << "getLastMessageArgI  : [" <<
        getLastMessageArgI(argCount) << "]\n";
    cout << "\n";

    cout << "LEFT_MARGIN             : [" << LEFT_MARGIN << "]\n";
    cout << "TOP_MARGIN              : [" << TOP_MARGIN << "]\n";
    cout << "BOTTOM_MARGIN           : [" << BOTTOM_MARGIN << "]\n";
    cout << "RIGHT_MARGIN            : [" << RIGHT_MARGIN << "]\n";
    cout << "\n";

    cout << "mMsgBoxTextareaWidth    : [" << mMsgBoxTextareaWidth << "]\n";
    cout << "mMsgBoxTextareaHeight   : [" << mMsgBoxTextareaHeight << "]\n";
    cout << "\n";

    cout << "mMsgBoxWindowWidth      : [" << mMsgBoxWindowWidth << "]\n";
    cout << "mMsgBoxWindowHeight     : [" << mMsgBoxWindowHeight << "]\n";
    cout << "\n";
}
