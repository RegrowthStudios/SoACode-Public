#include "stdafx.h"
#include "ImageLoader.h"
#include "lodepng.h"

#include <sys/types.h>
#include <sys/stat.h>

namespace vorb {
namespace core {
namespace graphics {

bool ImageLoader::loadPng(const cString imagepath, std::vector<ui8>& pixelStore, ui32& rWidth, ui32& rHeight, bool printError /* = true */) {
    FILE * file = fopen(imagepath, "rb");
    ui8* fileData2;
    size_t fileSize;

    if (!file) {
        if (printError) {
            perror(imagepath);
        }
        return false;
    }

    struct stat filestatus;
    stat(imagepath, &filestatus);

    fileSize = filestatus.st_size;
    std::vector <ui8> imgData(fileSize);

    fread(&(imgData[0]), 1, fileSize, file);
    fclose(file);

    unsigned error = lodepng::decode(pixelStore, rWidth, rHeight, imgData);

    //if there's an error, display it
    if (error) {
        std::cerr << "png decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return false;
    }
    return true;
}

}
}
}