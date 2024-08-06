
// App consts.
#define APP_NAME "xMsgBox"
#define APP_VERSION "2024-08-03.m"
#define APP_PARMS_REQUIRED 5

// Color styling.
#define COLOR_RED "\033[0;31m"
#define COLOR_BLUE "\033[1;34m"
#define COLOR_GREEN "\033[1;32m"
#define COLOR_NORMAL "\033[0m"

// MsgBox consts.
#define PIXELS_BETWEEN_MSGBOX_TEXTAREA_LINES 6

#define LEFT_MARGIN 100
#define RIGHT_MARGIN_FLUFF 4
#define RIGHT_MARGIN LEFT_MARGIN + RIGHT_MARGIN_FLUFF 
#define TOP_MARGIN 20
#define BOTTOM_MARGIN 32

// Debug.
#define DEBUG_IS_ON false

// Forward decls.
void displayVersion();
void displayUsage();

int getMaxMsgPixelWidth(int argCount, char** argValues);
int getMaxMsgPixelHeight(int argCount, char** argValues);

int getStringPixelWidth(string);
int getStringPixelHeight(string);

void drawMessageBoxOutLines(int argCount);

void drawMessageBox(int argCount, char** argValues);
long getYPosForMessageBoxLineIndex(long lineIndex,
    long lineHeight, long  lineSpace);

int getNumberOfMessages(int argCount);
int getFirstMessageArgI();
int getLastMessageArgI(int argCount);

void logDebugValues(int argCount);
