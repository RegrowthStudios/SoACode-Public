#include "stdafx.h"
#include "BlockTextureLoader.h"
#include "ModPathResolver.h"
#include "BlockTexturePack.h"
#include "BlockData.h"
#include "Errors.h"

#include <Vorb/graphics/ImageIO.h>

// Used for error checking
#define CONNECTED_WIDTH 12
#define CONNECTED_HEIGHT 4
#define GRASS_WIDTH 3
#define GRASS_HEIGHT 3
#define HORIZONTAL_WIDTH 4
#define HORIZONTAL_HEIGHT 1

void BlockTextureLoader::init(ModPathResolver* texturePathResolver, BlockTexturePack* texturePack) {
    m_texturePathResolver = texturePathResolver;
    m_texturePack = texturePack;
}

void BlockTextureLoader::loadBlockTextures(Block& block) {
    // Default values for texture indices
    for (i32 i = 0; i < 6; i++) {
        block.base[i] = 0;
        block.normal[i] = 0;
        block.overlay[i] = 0;
    }

    // Load the textures for each face
    for (int i = 0; i < 6; i++) {
        block.textures[i] = loadTexture(block.texturePaths[i]); // TODO(Ben): Free the string?
        if (block.textures[i]) {
            block.base[i] = block.textures[i]->base.index;
            block.overlay[i] = block.textures[i]->overlay.index;
        } else {
            block.textures[i] = m_texturePack->getDefaultTexture();
            printf("Warning: Could not load texture %s for block %s\n", block.texturePaths[i].c_str(), block.name.c_str());
        }
    }
   
    // TODO(Ben): NOPE
    /* BlockTexture particleTexture;
     GameManager::texturePackLoader->getBlockTexture(particleTexName, particleTexture);
     particleTex = particleTexture.base.index;*/

    // Calculate flora height
    // TODO(Ben): This is dubious
    if (block.textures[0]->base.method == ConnectedTextureMethods::FLORA) {
        // Just a bit of algebra to solve for n with the equation y = (n^2 + n) / 2
        // which becomes n = (sqrt(8 * y + 1) - 1) / 2
        int y = block.textures[0]->base.size.y;
        block.floraHeight = (ui16)(sqrt(8 * y + 1) - 1) / 2;
    }
}

BlockTexture* BlockTextureLoader::loadTexture(const nString& filePath) {
    // Check for cached texture
    BlockTexture* exists = m_texturePack->findTexture(filePath);
    if (exists) return exists;

    // Resolve the path
    vio::Path path;
    if (!m_texturePathResolver->resolvePath(filePath, path)) return nullptr;

    // Get next free texture
    BlockTexture* texture = m_texturePack->getNextFreeTexture();

    // Load the tex file (if it exists)
    loadTexFile(path.getString(), texture);

    // If there wasn't an explicit override base path, we use the texturePath
    if (texture->base.path.empty()) texture->base.path = path.getString();

    // Load the layers
    loadLayer(texture->base);
    if (texture->overlay.path.size()) {
        loadLayer(texture->overlay);
    }

    return texture;
}

bool BlockTextureLoader::loadLayer(BlockTextureLayer& layer) {
    AtlasTextureDescription desc = m_texturePack->findLayer(layer.path);
    // Check if its already been loaded
    if (desc.size.x != 0) {
        // Already loaded so just use the desc
        // TODO(Ben): Worry about different methods using same file?
        layer.size = desc.size;
        layer.index = desc.index;
    } else {
        // Get pixels for the base texture
        vg::ScopedBitmapResource rs = vg::ImageIO().load(layer.path, vg::ImageIOFormat::RGBA_UI8);
        // Do post processing on the layer
        if (!postProcessLayer(rs, layer)) return false;

        m_texturePack->addLayer(layer, (color4*)rs.bytesUI8v4);
    }
    return true;
}

bool BlockTextureLoader::loadTexFile(const nString& imagePath, BlockTexture* texture) {
    // Convert .png to .tex
    nString texFileName = imagePath;
    texFileName.replace(texFileName.end() - 4, texFileName.end(), ".tex");

    nString data;
    m_iom.readFileToString(texFileName.c_str(), data);
    if (data.length()) {
        if (keg::parse(texture, data.c_str(), "BlockTexture") == keg::Error::NONE) {
            if (texture->base.weights.size() > 0) {
                texture->base.totalWeight = 0;
                for (size_t i = 0; i < texture->base.weights.size(); i++) {
                    texture->base.totalWeight += texture->base.weights[i];
                }
            }
            if (texture->overlay.weights.size() > 0) {
                texture->overlay.totalWeight = 0;
                for (size_t i = 0; i < texture->overlay.weights.size(); i++) {
                    texture->overlay.totalWeight += texture->overlay.weights[i];
                }
            }

            // Get ColorMaps
            /* if (texture->base.useMapColor.length()) {
                 texture->base.colorMap = getColorMap(texture->base.useMapColor);
                 }
                 if (texture->overlay.useMapColor.length()) {
                 texture->overlay.colorMap = getColorMap(texture->overlay.useMapColor);
                 }*/
            return true;
        }
    }
    return false;
}

bool BlockTextureLoader::postProcessLayer(vg::ScopedBitmapResource& bitmap, BlockTextureLayer& layer) {

    ui32 floraRows;
    const ui32& resolution = m_texturePack->getResolution();

    // Helper for checking dimensions
#define DIM_CHECK(w, cw, h, ch, method) \
    if (bitmap.width != resolution * cw) { \
        pError("Texture " + layer.path + " is " #method " but width is not " + std::to_string(cw)); \
        return false; \
            } \
    if (bitmap.height != resolution * ch) {  \
        pError("Texture " + layer.path + " is " #method " but height is not " + std::to_string(ch)); \
        return false; \
            }

    // Pixels must exist
    if (!bitmap.data) return false;

    // Check that the texture is sized in units of resolution
    if (bitmap.width % resolution) {
        pError("Texture " + layer.path + " width must be a multiple of " + std::to_string(resolution));
        return false;
    }
    if (bitmap.height % resolution) {
        pError("Texture " + layer.path + " height must be a multiple of " + std::to_string(resolution));
        return false;
    }

    // Check for errors and postprocessing based on method
    switch (layer.method) {
        // Need to set up numTiles and totalWeight for RANDOM method
        case ConnectedTextureMethods::CONNECTED:
            DIM_CHECK(width, CONNECTED_WIDTH, bitmap.height, CONNECTED_HEIGHT, CONNECTED);
            break;
        case ConnectedTextureMethods::RANDOM:
            layer.numTiles = bitmap.width / bitmap.height;
            if (layer.weights.size() == 0) {
                layer.totalWeight = layer.numTiles;
            } else { // Need to check if there is the right number of weights
                if (layer.weights.size() * resolution != bitmap.width) {
                    pError("Texture " + layer.path + " weights length must match number of columns or be empty. weights.length() = " +
                           std::to_string(layer.weights.size()) + " but there are " + std::to_string(bitmap.width / resolution) + " columns.");
                    return false;
                }
            }
            break;
        case ConnectedTextureMethods::GRASS:
            DIM_CHECK(width, GRASS_WIDTH, bitmap.height, GRASS_HEIGHT, GRASS);
            break;
        case ConnectedTextureMethods::HORIZONTAL:
            DIM_CHECK(width, HORIZONTAL_WIDTH, bitmap.height, HORIZONTAL_HEIGHT, HORIZONTAL);
            break;
        case ConnectedTextureMethods::VERTICAL:
            DIM_CHECK(width, HORIZONTAL_HEIGHT, bitmap.height, HORIZONTAL_WIDTH, VERTICAL);
            break;
        case ConnectedTextureMethods::REPEAT:
            DIM_CHECK(width, layer.size.x, bitmap.height, layer.size.y, REPEAT);
            break;
        case ConnectedTextureMethods::FLORA:
            floraRows = BlockTextureLayer::getFloraRows(layer.floraHeight);
            if (bitmap.height != resolution * floraRows) {
                pError("Texture " + layer.path + " texture height must be equal to (maxFloraHeight^2 + maxFloraHeight) / 2 * resolution = " +
                       std::to_string(bitmap.height) + " but it is " + std::to_string(resolution * floraRows));
                return false;
            }
            // If no weights, they are all equal
            if (layer.weights.size() == 0) {
                layer.totalWeight = bitmap.width / resolution;
            } else { // Need to check if there is the right number of weights
                if (layer.weights.size() * resolution != bitmap.width) {
                    pError("Texture " + layer.path + " weights length must match number of columns or be empty. weights.length() = " +
                           std::to_string(layer.weights.size()) + " but there are " + std::to_string(bitmap.width / resolution) + " columns.");
                    return false;
                }
            }
            // Tile dimensions and count
            layer.size.x = bitmap.width / resolution;
            layer.size.y = floraRows;
            layer.numTiles = layer.size.x * layer.size.y;
            break;
        case ConnectedTextureMethods::NONE:
            DIM_CHECK(bitmap.width, 1, bitmap.height, 1, NONE);
            break;
        default:
            break;
    }

    layer.initBlockTextureFunc();
    return true;
}