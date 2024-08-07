
// Std C and c++.
#include <iostream>
#include <list>
#include <string>
#include <string.h>

// X11.
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

// Application.
#include "xMsgBox.h"


/** ********************************************************
 ** Module globals and consts.
 **/
// xMsgBox globals.
int mMsgBoxXPos;
int mMsgBoxYPos;

string mMsgBoxTitle;

int mMsgBoxTextareaWidth;
int mMsgBoxTextareaHeight;

int mMsgBoxWindowWidth;
int mMsgBoxWindowHeight;

std::list<string> mMsgBoxLines;

// Module globals.
Display* mDisplay;

Window mMsgBox;

XftFont* mFont;

const XftColor mFontColor = { .pixel = 0x0, .color = { 
    .red = 0xff, .green = 0xff,
    .blue = 0xff, .alpha = 0xffff } };

/** ********************************************************
 ** Module Entry.
 **/
int main(int argCount, char** argValues) {
    // Ensure proper invocation.
    if (argCount < APP_PARMS_REQUIRED) {
        displayVersion();
        displayUsage();
        exit(1);
    }

    // Set xMsgBox globals from user.
    mMsgBoxXPos = atoi(argValues[1]);
    mMsgBoxYPos = atoi(argValues[2]);
    mMsgBoxTitle += argValues[3];

    // Open X11, ensure it's available.
    mDisplay = XOpenDisplay(NULL);
    if (mDisplay == NULL) {
        cout << COLOR_RED << "\nMsgBox: X11 Windows are "
            "unavailable with this desktop - FATAL.\n" << COLOR_NORMAL;
        exit(2);
    }

    // Set font for layouts. // -misc-fixed-medium-r-normal
    mFont = XftFontOpenName(mDisplay, DefaultScreen(mDisplay), "");
    if (mFont == NULL) {
        cout << COLOR_RED << "\nMsgBox: Cannot open font - "
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

    // Set xMsgBox title string.
    XTextProperty properties;
    properties.value = (unsigned char*) mMsgBoxTitle.c_str();

    properties.encoding = XA_STRING;
    properties.format = 8;
    properties.nitems = mMsgBoxTitle.length();
    XSetWMName(mDisplay, mMsgBox, &properties);

    // Set xMsgBox icon.
    char* appName = strdup("msgbox");
    char* iconName = mMsgBoxTitle == "Error" ? strdup("xmsgboxerror") :
        mMsgBoxTitle == "Warning" ? strdup("xmsgboxwarning") :
        strdup("xmsgboxinfo");

    XClassHint* classHint = XAllocClassHint();
    if (classHint) {
        classHint->res_class = iconName;
        classHint->res_name = appName;
        XSetClassHint(mDisplay, mMsgBox, classHint);
    }
    XTextProperty iconProperty;
    XStringListToTextProperty(&iconName, 1, &iconProperty);
    XSetWMIconName(mDisplay, mMsgBox, &iconProperty);

    // Map (show) xMsgBox window.
    XMapWindow(mDisplay, mMsgBox);
    XMoveWindow(mDisplay, mMsgBox, mMsgBoxXPos, mMsgBoxYPos);

    // Select observable x11-events.
    XSelectInput(mDisplay, mMsgBox, ExposureMask);

    // Select observable x11-client messages.
    Atom mDeleteMsg = XInternAtom(mDisplay,
        "WM_DELETE_WINDOW", False);
    XSetWMProtocols(mDisplay, mMsgBox, &mDeleteMsg, 1);

    // Loop until close event frees us.
    bool msgboxActive = true;

    while (msgboxActive) {
        // Grab next event to examine.
        XEvent event;
        XNextEvent(mDisplay, &event);

        // Process ClientMsg Close event.
        if (event.type == ClientMessage) {
            if (event.xclient.data.l[0] == mDeleteMsg) {
                msgboxActive = false;
            }
            break;
        }

        // Process Expose event. Set the xMsgBox
        // Msg on Window expose.
        if (event.type == Expose) {
            if (DEBUG_IS_ON) {
                drawMessageBoxOutLines(argCount);
            }
            drawMessageBox(argCount, argValues);
        }
    }

    // Close display & done.
    XUnmapWindow(mDisplay, mMsgBox);
    XDestroyWindow(mDisplay, mMsgBox);
    XCloseDisplay(mDisplay);
}

/** ********************************************************
 ** This method displays the xMsgBox version string.
 **/
void displayVersion() {
    cout << COLOR_BLUE << "\n" << APP_VERSION <<
        COLOR_NORMAL << "\n";
}

/** ********************************************************
 ** This method displays the basic use syntax.
 **/
void displayUsage() {
    cout << COLOR_BLUE << "\nUseage:" << COLOR_NORMAL << "\n";
    cout << COLOR_GREEN << "   " << APP_NAME << " xPos yPos "
        "title message message2 message3 ..." <<
        COLOR_NORMAL << "\n";

    cout << COLOR_BLUE << "\nExample:" << COLOR_NORMAL << "\n";
    cout << COLOR_GREEN << "   " << APP_NAME << " 600 400 \"Error\" "
        "\"Something failed.\" \"Please try later.\"" <<
        COLOR_NORMAL << "\n";
}

/** ********************************************************
 ** This method returns pixel witdh of a text string.
 **/
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

/** ********************************************************
 ** This method returns pixel height of a textSring.
 **/
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

/** ********************************************************
 ** This method returns pixel witdh of a text string.
 **/
int getStringPixelWidth(string textString) {
    XGlyphInfo textExtents;
    XftTextExtents8(mDisplay, mFont, (const FcChar8*)
        textString.c_str(), textString.length(), &textExtents);

    return textExtents.width;
}

/** ********************************************************
 ** This method returns pixel height of a text string.
 **/
int getStringPixelHeight(string textString) {
    XGlyphInfo textExtents;
    XftTextExtents8(mDisplay, mFont, (const FcChar8*)
        textString.c_str(), textString.length(), &textExtents);

    return textExtents.height;
}

/** ********************************************************
 ** This method draws an visual outline of the xMsgBox
 ** for debugging. S/b safe to remove / deprecate.
 **/
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

/** ********************************************************
 ** Draw all message strings centered in the window.
 **/
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

/** ********************************************************
 ** This method determines pixel offset for a message
 ** string drawline.
 */
long getYPosForMessageBoxLineIndex(long lineIndex,
    long lineHeight, long lineSpace) {

    return ((lineIndex + 1) * (lineHeight + lineSpace));
}

/** ********************************************************
 ** This method returns the number of message strings that
 ** xMsgBox is to display from the users cmdline.
 **/
int getNumberOfMessages(int argCount) {
    return getLastMessageArgI(argCount) -
        getFirstMessageArgI() + 1;
}

/** ********************************************************
 ** This method returns the argi of argv[] of the First
 ** message string that xMsgBox is to display from
 ** the users cmdline.
 **/
int getFirstMessageArgI() {
    return APP_PARMS_REQUIRED - 1;
}

/** ********************************************************
 ** This method returns the argi of argv[] of the Last
 ** message string that xMsgBox is to display from
 ** the users cmdline.
 **/
int getLastMessageArgI(int argCount) {
    return argCount - 1;
}

/** ********************************************************
 ** This method logs critical info for devs.
 **/
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
