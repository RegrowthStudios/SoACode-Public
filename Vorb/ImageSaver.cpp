#include "stdafx.h"
#include "ImageSaver.h"

#include "lodepng.h"

namespace vorb {
namespace core {
namespace graphics {

bool ImageSaver::savePng(nString fileName, ui32 width, ui32 height, std::vector<ui8> imgData) {
    unsigned error = lodepng::encode(fileName, imgData, width, height);

    //if there's an error, display it
    if (error) {
        std::cerr << "png encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
        return false;
    }
    return true;
}

}
}
}