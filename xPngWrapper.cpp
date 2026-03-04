
/**
 * Minimally wrap a PNG file with x11 accessors.
 */

// Std C and c++.
#include <fstream>
#include <iostream>
#include <png.h>
#include <string>
#include <stdexcept>
#include <vector>

using namespace std;

// X11.
#include <X11/extensions/Xrender.h>

// Application.
#include "xPngWrapper.h"


/**
 * Class instantiation.
 */
xPngWrapper::xPngWrapper() :
    mPngFileName("") {
}

/**
 * Class instantiation.
 */
xPngWrapper::xPngWrapper(string pngFileName) :
    mPngFileName(pngFileName) {
    if (pngFileName == "") {
        mErrorStatus = "The input "
            "file name is not specified.";
        return;
    }

    FILE* pngFile = fopen(pngFileName.c_str(), "rb");
    if (!pngFile) {
        mErrorStatus = "The input "
            "file does not exist.";
        return;
    }

    png_byte pngFileHeader[8];
    fread(pngFileHeader, 1, 8, pngFile);
    if (png_sig_cmp(pngFileHeader, 0, 8)) {
        fclose(pngFile);
        mErrorStatus = "The input "
            "file is not a valid PNG file.";
        return;
    }

    png_structp pngFileReadStruct =
        png_create_read_struct(PNG_LIBPNG_VER_STRING,
            nullptr, nullptr, nullptr);
    if (!pngFileReadStruct) {
        fclose(pngFile);
        mErrorStatus = "libpng "
            "cannot be initialized to handle "
            "the input file.";
        return;
    }

    png_infop pngFileInfoStruct =
        png_create_info_struct(pngFileReadStruct);
    if (!pngFileInfoStruct) {
        png_destroy_read_struct(&pngFileReadStruct,
            nullptr, nullptr);
        fclose(pngFile);
        mErrorStatus = "The input file "
            "cannot be opened by libpng.";
        return;
    }

    if (setjmp(png_jmpbuf(pngFileReadStruct))) {
        png_destroy_read_struct(&pngFileReadStruct,
            &pngFileInfoStruct, nullptr);
        fclose(pngFile);
        mErrorStatus = "The input file "
            "cannot cannot be read by libpng.";
        return;
    }

    png_init_io(pngFileReadStruct, pngFile);
    png_set_sig_bytes(pngFileReadStruct, 8);
    png_read_info(pngFileReadStruct, pngFileInfoStruct);

    mWidth = png_get_image_width(pngFileReadStruct,
        pngFileInfoStruct);
    mHeight = png_get_image_height(pngFileReadStruct,
        pngFileInfoStruct);
    mColorType = png_get_color_type(pngFileReadStruct,
        pngFileInfoStruct);
    mBitDepth = png_get_bit_depth(pngFileReadStruct,
        pngFileInfoStruct);

    // Set up transformations for a consistent
    // RGBA 8-bit format.
    if (mBitDepth == 16) {
        cout << COLOR_YELLOW << "xPngWrapper: "
            "Downconverting from 16bit to 8bit." <<
            COLOR_NORMAL << "\n";
        png_set_strip_16(pngFileReadStruct);
    }

    if (mColorType == PNG_COLOR_TYPE_PALETTE) {
        cout << COLOR_YELLOW << "xPngWrapper: "
            "Setting palette to rgb." <<
            COLOR_NORMAL << "\n";
        png_set_palette_to_rgb(pngFileReadStruct);
    }

    if (mColorType == PNG_COLOR_TYPE_GRAY &&
        mBitDepth < 8) {
        cout << COLOR_YELLOW << "xPngWrapper: "
            "Expanding Gray (1,2,4) to (8)." <<
            COLOR_NORMAL << "\n";
        png_set_expand_gray_1_2_4_to_8(
        pngFileReadStruct);
    }

    if (png_get_valid(pngFileReadStruct,
        pngFileInfoStruct, PNG_INFO_tRNS)) {
        cout << COLOR_YELLOW << "xPngWrapper: "
            "Setting tRNS to Alpha." <<
            COLOR_NORMAL << "\n";
        png_set_tRNS_to_alpha(pngFileReadStruct);
    }

    if (mColorType == PNG_COLOR_TYPE_RGB ||
        mColorType == PNG_COLOR_TYPE_GRAY ||
        mColorType == PNG_COLOR_TYPE_PALETTE) {
        cout << COLOR_YELLOW << "xPngWrapper: "
            "Setting filler." <<
            COLOR_NORMAL << "\n";
        png_set_filler(pngFileReadStruct, 0xFF,
            PNG_FILLER_AFTER);
    }

    if (mColorType == PNG_COLOR_TYPE_GRAY ||
        mColorType == PNG_COLOR_TYPE_GRAY_ALPHA) {
        cout << COLOR_YELLOW << "xPngWrapper: "
            "Setting gray to RGB." <<
            COLOR_NORMAL << "\n";
        png_set_gray_to_rgb(pngFileReadStruct);
    }

    // First two values are array w/h.
    mPngData.push_back(getWidth());
    mPngData.push_back(getHeight());

    // Update libpng info, the get Png
    // rowbytes & allocate a row buffer.
    png_read_update_info(pngFileReadStruct,
        pngFileInfoStruct);
    png_uint_32 rowBytes = png_get_rowbytes(
        pngFileReadStruct, pngFileInfoStruct);
    vector<png_byte> pngRowData(rowBytes);

    // Copy the PNG data into the Icon.
    for (png_uint_32 h = 0; h < getHeight(); ++h) {
        png_read_row(pngFileReadStruct,
            pngRowData.data(), nullptr);
        for (png_uint_32 x = 0;
            x < rowBytes; x += 4) {
            const XRenderColor rcRGBA = {
                .red = pngRowData[x + 0],
                .green = pngRowData[x + 1],
                .blue = pngRowData[x + 2],
                .alpha = pngRowData[x + 3]
            };
            const unsigned long COLOR_PIXEL =
                (static_cast<unsigned long>
                    (rcRGBA.alpha) << 24) |
                (static_cast<unsigned long>
                    (rcRGBA.red)   << 16) |
                (static_cast<unsigned long>
                    (rcRGBA.green) <<  8) |
                (static_cast<unsigned long>
                    (rcRGBA.blue)  <<  0);
            mPngData.push_back(COLOR_PIXEL);
        }
    }

    // Cleanup, exit.
    png_read_end(pngFileReadStruct, nullptr);
    png_destroy_read_struct(&pngFileReadStruct,
        &pngFileInfoStruct, nullptr);
    fclose(pngFile);
}

xPngWrapper::~xPngWrapper() {
}

/**
 * Class getters.
 */
int xPngWrapper::getWidth() {
    return mWidth;
}

int xPngWrapper::getHeight() {
    return mHeight;
}

int xPngWrapper::getColorType() {
    return mColorType;
}

int xPngWrapper::getBitDepth() {
    return mBitDepth;
}

bool xPngWrapper::hasErrorStatus() {
    return mErrorStatus != "";
}
string xPngWrapper::errorStatus() {
    return mErrorStatus;
}

vector<unsigned long> 
xPngWrapper::getPngData() {
    return mPngData;
}