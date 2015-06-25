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

void BlockTextureLoader::loadTextureData() {
    if (!loadLayerProperties()) pError("Failed to load LayerProperties.yml");
    if (!loadTextureProperties()) pError("Failed to load Textures.yml");
    if (!loadBlockTextureMapping()) pError("Failed to load BlockTextureMapping.yml");
}

void BlockTextureLoader::loadBlockTextures(Block& block) {
    // Default values for texture indices
    for (i32 i = 0; i < 6; i++) {
        block.base[i] = 0;
        block.normal[i] = 0;
        block.overlay[i] = 0;
    }

    // Check for block mapping
    auto& it = m_blockMappings.find(block.sID);
    if (it == m_blockMappings.end()) {
        printf("Warning: Could not load texture mapping for block %s\n", block.sID);
        for (int i = 0; i < 6; i++) {
            block.textures[i] = m_texturePack->getDefaultTexture();
        }
        return;
    }

    // Load the textures for each face
    BlockTextureNames& names = it->second;
    for (int i = 0; i < 6; i++) {
        BlockTexture* texture = m_texturePack->findTexture(names.names[i]);
        if (texture) {
            loadLayer(texture->base);
            if (texture->overlay.path.size()) {
                loadLayer(texture->overlay);
            }
            block.textures[i] = texture;
        } else {
            printf("Warning: Could not load texture %d for block %s\n", i, block.sID);
            block.textures[i] = m_texturePack->getDefaultTexture();
            return;
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

bool BlockTextureLoader::loadLayerProperties() {
    vio::Path path;
    if (!m_texturePathResolver->resolvePath("LayerProperties.yml", path)) return nullptr;

    // Read file
    nString data;
    m_iom.readFileToString(path, data);
    if (data.empty()) return false;

    // Convert to YAML
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        context.reader.dispose();
        return false;
    }

    // Load all layers
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        BlockTextureLayer layer;

        // Load data
        keg::parse((ui8*)&layer, value, context, &KEG_GLOBAL_TYPE(BlockTextureLayer));
 
        // Cache the layer
        m_layers[key] = layer;
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();

    return true;
}

bool BlockTextureLoader::loadTextureProperties() {
    vio::Path path;
    if (!m_texturePathResolver->resolvePath("Textures.yml", path)) return nullptr;

    // Read file
    nString data;
    m_iom.readFileToString(path, data);
    if (data.empty()) return false;

    // Convert to YAML
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        context.reader.dispose();
        return false;
    }

    BlockTexture* texture;
    auto valf = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        
        if (key == "base") {
            if (keg::getType(value) == keg::NodeType::MAP) {
                // TODO(Ben): Handle map
            } else {
                nString base = keg::convert<nString>(value);
                auto& it = m_layers.find(base);
                if (it != m_layers.end()) {
                    texture->base = it->second;
                }
            }
        } else if (key == "overlay") {
            if (keg::getType(value) == keg::NodeType::MAP) {
                // TODO(Ben): Handle map
            } else {
                nString overlay = keg::convert<nString>(value);
                auto& it = m_layers.find(overlay);
                if (it != m_layers.end()) {
                    texture->overlay = it->second;
                }
            }
        } else if (key == "blendMode") {
            nString v = keg::convert<nString>(value);
            if (v == "add") {
                texture->blendMode = BlendType::ADD;
            } else if (v == "multiply") {
                texture->blendMode = BlendType::MULTIPLY;
            } else if (v == "subtract") {
                texture->blendMode = BlendType::SUBTRACT;
            }
        }
    });

    // Load all layers
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        texture = m_texturePack->getNextFreeTexture(key);
        context.reader.forAllInMap(value, valf);
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();
    delete valf;

    return true;
}

nString getBlockTextureName(const keg::Node& value) {
    if (keg::getType(value) == keg::NodeType::MAP) {
        // TODO(Ben): Handle map
    } else {
        return keg::convert<nString>(value);
    }
}

bool BlockTextureLoader::loadBlockTextureMapping() {

    vio::Path path;
    if (!m_texturePathResolver->resolvePath("BlockTextureMapping.yml", path)) return nullptr;

    // Read file
    nString data;
    m_iom.readFileToString(path, data);
    if (data.empty()) return false;

    // Convert to YAML
    keg::ReadContext context;
    context.env = keg::getGlobalEnvironment();
    context.reader.init(data.c_str());
    keg::Node node = context.reader.getFirst();
    if (keg::getType(node) != keg::NodeType::MAP) {
        context.reader.dispose();
        return false;
    }

    BlockTextureNames* names;
    auto valf = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        nString name = getBlockTextureName(value);
        if (key == "texture") {
            for (int i = 0; i < 6; i++) {
                names->names[i] = name;
            }
        } else if (key == "textureOpX") {
            names->names[(int)vvox::Cardinal::X_NEG] = name;
            names->names[(int)vvox::Cardinal::X_POS] = name;
        } else if (key == "textureOpY") {
            names->names[(int)vvox::Cardinal::Y_NEG] = name;
            names->names[(int)vvox::Cardinal::Y_POS] = name;
        } else if (key == "textureOpZ") {
            names->names[(int)vvox::Cardinal::Z_NEG] = name;
            names->names[(int)vvox::Cardinal::Z_POS] = name;
        } else if (key == "textureTop") {
            names->names[(int)vvox::Cardinal::Y_POS] = name;
        } else if (key == "textureBottom") {
            names->names[(int)vvox::Cardinal::Y_NEG] = name;
        } else if (key == "textureFront") {
            names->names[(int)vvox::Cardinal::Z_POS] = name;
        } else if (key == "textureBack") {
            names->names[(int)vvox::Cardinal::Z_NEG] = name;
        } else if (key == "textureLeft") {
            names->names[(int)vvox::Cardinal::X_NEG] = name;
        } else if (key == "textureRight") {
            names->names[(int)vvox::Cardinal::X_POS] = name;
        }
    });

    // Load all layers
    auto f = makeFunctor<Sender, const nString&, keg::Node>([&](Sender, const nString& key, keg::Node value) {
        BlockTextureNames tNames = {};
        names = &tNames;
        context.reader.forAllInMap(value, valf);
        m_blockMappings[key] = tNames;
    });
    context.reader.forAllInMap(node, f);
    delete f;
    context.reader.dispose();
    delete valf;

    return true;
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
        vio::Path path;
        if (!m_texturePathResolver->resolvePath(layer.path, path)) return nullptr;
        // Get pixels for the base texture
        vg::ScopedBitmapResource rs = vg::ImageIO().load(path, vg::ImageIOFormat::RGBA_UI8);
        // Do post processing on the layer
        if (!postProcessLayer(rs, layer)) return false;

        m_texturePack->addLayer(layer, (color4*)rs.bytesUI8v4);
    }
    return true;
}

bool BlockTextureLoader::postProcessLayer(vg::ScopedBitmapResource& bitmap, BlockTextureLayer& layer) {

    ui32 floraRows;
    const ui32& resolution = m_texturePack->getResolution();

    layer.size.x = bitmap.width / resolution;
    layer.size.y = bitmap.height / resolution;
    // TODO(Ben): Floraheight

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
        //case ConnectedTextureMethods::FLORA:
        //    floraRows = BlockTextureLayer::getFloraRows(layer.floraHeight);
        //    if (bitmap.height != resolution * floraRows) {
        //        pError("Texture " + layer.path + " texture height must be equal to (maxFloraHeight^2 + maxFloraHeight) / 2 * resolution = " +
        //               std::to_string(bitmap.height) + " but it is " + std::to_string(resolution * floraRows));
        //        return false;
        //    }
        //    // If no weights, they are all equal
        //    if (layer.weights.size() == 0) {
        //        layer.totalWeight = bitmap.width / resolution;
        //    } else { // Need to check if there is the right number of weights
        //        if (layer.weights.size() * resolution != bitmap.width) {
        //            pError("Texture " + layer.path + " weights length must match number of columns or be empty. weights.length() = " +
        //                   std::to_string(layer.weights.size()) + " but there are " + std::to_string(bitmap.width / resolution) + " columns.");
        //            return false;
        //        }
        //    }
        //    // Tile dimensions and count
        //    layer.size.x = bitmap.width / resolution;
        //    layer.size.y = floraRows;
        //    layer.numTiles = layer.size.x * layer.size.y;
        //    break;
        case ConnectedTextureMethods::NONE:
            DIM_CHECK(bitmap.width, 1, bitmap.height, 1, NONE);
            break;
        default:
            break;
    }

    layer.initBlockTextureFunc();
    return true;
}