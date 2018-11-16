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
    // Check for block mapping
    auto it = m_blockMappings.find(block.sID);
    if (it == m_blockMappings.end()) {
        printf("Warning: Could not load texture mapping for block %s\n", block.sID.c_str());
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
            loadLayer(texture->layers.base);
            if (texture->layers.overlay.path.size()) {
                loadLayer(texture->layers.overlay);
            }
            block.textures[i] = texture;
        } else {
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
    if (block.textures[0]->layers.base.method == ConnectedTextureMethods::FLORA) {
        // Just a bit of algebra to solve for n with the equation y = (n^2 + n) / 2
        // which becomes n = (sqrt(8 * y + 1) - 1) / 2
        int y = block.textures[0]->layers.base.size.y;
        block.floraHeight = (ui16)(sqrt(8 * y + 1) - 1) / 2;
    }
}

bool BlockTextureLoader::loadLayerProperties() {
    vio::Path path;
    if (!m_texturePathResolver->resolvePath("LayerProperties.yml", path)) return false;

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

    // Layer handle for lf
    BlockTextureLayer* lp;
    // Custom values for parsing
    keg::Value colorVal = keg::Value::basic(0, keg::BasicType::UI8_V3);
    keg::Value methodVal = keg::Value::custom(0, "ConnectedTextureMethods", true);
 
    // Custom layer parse
    auto lf = makeFunctor([&, this](Sender, const nString& key, keg::Node value) {
        if (key == "path") {
            lp->path = keg::convert<nString>(value);
        } else if (key == "normalMap") {
            lp->normalPath = keg::convert<nString>(value);
        } else if (key == "dispMap") {
            lp->dispPath = keg::convert<nString>(value);
        } else if (key == "color") {
            switch (keg::getType(value)) {
                case keg::NodeType::VALUE:
                    lp->colorMapPath = keg::convert<nString>(value);
                    lp->colorMap = this->getTexturePack()->getColorMap(lp->colorMapPath);
                    break;
                case keg::NodeType::SEQUENCE:
                    keg::evalData((ui8*)&lp->color, &colorVal, value, context);
                    break;
                default:
                    break;
            }
        } else if (key == "altColors") {
            // TODO(Ben): Implement
        } else if (key == "method") {
            keg::evalData((ui8*)&lp->method, &methodVal, value, context);
        } else if (key == "coupling") {
            // TODO(Ben): Implement
        }
    });
    // Load all layers
    auto f = makeFunctor([&](Sender, const nString& key, keg::Node value) {
        BlockTextureLayer layer;
        lp = &layer;

        // Manual parse
        context.reader.forAllInMap(value, &lf);

        // Cache the layer
        m_layers[key] = layer;
    });
    context.reader.forAllInMap(node, &f);
    context.reader.dispose();

    return true;
}

// For parsing a block texture
#define TEXTURE_PARSE_CODE \
if (key == "base") { \
    if (keg::getType(value) == keg::NodeType::MAP) { \
    } else { \
        nString base = keg::convert<nString>(value); \
       auto& it = m_layers.find(base); \
       if (it != m_layers.end()) { \
            texture->base = it->second; \
       } \
    } \
} else if (key == "overlay") { \
    if (keg::getType(value) == keg::NodeType::MAP) { \
    } else { \
      nString overlay = keg::convert<nString>(value); \
      auto& it = m_layers.find(overlay); \
        if (it != m_layers.end()) { \
            texture->overlay = it->second; \
        } \
    } \
} else if (key == "blendMode") { \
    nString v = keg::convert<nString>(value); \
    if (v == "add") { \
        texture->blendMode = BlendType::ADD; \
    } else if (v == "multiply") { \
    texture->blendMode = BlendType::MULTIPLY; \
    } else if (v == "subtract") { \
    texture->blendMode = BlendType::SUBTRACT; \
    } \
} \

bool BlockTextureLoader::loadTextureProperties() {
    vio::Path path;
    if (!m_texturePathResolver->resolvePath("Textures.yml", path)) return false;

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
    auto valf = makeFunctor([&](Sender, const nString& key, keg::Node value) {
//        TEXTURE_PARSE_CODE;
        if(key=="base")
        {
            if(keg::getType(value)==keg::NodeType::MAP)
            {
            }
            else
            {
                nString base=keg::convert<nString>(value);
                auto it=m_layers.find(base);
                if(it!=m_layers.end())
                {
                    texture->layers.base = it->second;
                }
            }
        }
        else if(key=="overlay")
        {
            if(keg::getType(value)==keg::NodeType::MAP)
            {
            }
            else
            {
                nString overlay=keg::convert<nString>(value);
                auto it=m_layers.find(overlay);
                if(it!=m_layers.end())
                {

                    texture->layers.overlay = it->second;
                }
            }
        }
        else if(key=="blendMode")
        {
            nString v=keg::convert<nString>(value);
            if(v=="add")
            {
                texture->blendMode=BlendType::ADD;
            }
            else if(v=="multiply")
            {
                texture->blendMode=BlendType::MULTIPLY;
            }
            else if(v=="subtract")
            {
                texture->blendMode=BlendType::SUBTRACT;
            }
        }
    });

    // Load all layers
    auto f = makeFunctor([&](Sender, const nString& key, keg::Node value) {
        texture = m_texturePack->getNextFreeTexture(key);
        context.reader.forAllInMap(value, &valf);
    });
    context.reader.forAllInMap(node, &f);
    context.reader.dispose();

    return true;
}

bool BlockTextureLoader::loadBlockTextureMapping() {

    vio::Path path;
    if (!m_texturePathResolver->resolvePath("BlockTextureMapping.yml", path)) return false;

    // Read file
    nString data;
    const nString* blockName;
    BlockTexture* texture;
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

    // For parsing textures
    auto texParseFunctor = makeFunctor([&](Sender, const nString& key, keg::Node value) {
        if (key == "base") {
            if (keg::getType(value) == keg::NodeType::MAP) {

            } else {

                nString base = keg::convert<nString>(value);
                auto it = m_layers.find(base);
                if (it != m_layers.end()) {
                    texture->layers.base = it->second;
                }
            }
        } else if (key == "overlay") {
            if (keg::getType(value) == keg::NodeType::MAP) {

            } else {
                nString overlay = keg::convert<nString>(value);
                auto it = m_layers.find(overlay);
                if (it != m_layers.end()) {

                    texture->layers.overlay = it->second;
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

    nString *texNames;

    auto valParseFunctor = makeFunctor([&](Sender, const nString& key, keg::Node value) {
        nString name;
        // Conditional parsing, map vs value
        if (keg::getType(value) == keg::NodeType::MAP) {
            name = *blockName + std::to_string(m_generatedTextureCounter++);
            texture = m_texturePack->getNextFreeTexture(name);
            context.reader.forAllInMap(value, &texParseFunctor);
        } else {
            name = keg::convert<nString>(value);
        }

        if (key == "texture") {
            texNames[0] = name;
        } else if (key == "textureOpX") {
            texNames[1] = name;
        } else if (key == "textureOpY") {
            texNames[2] = name;
        } else if (key == "textureOpZ") {
            texNames[3] = name;
        } else if (key == "textureTop") {
            texNames[4] = name;
        } else if (key == "textureBottom") {
            texNames[5] = name;
        } else if (key == "textureFront") {
            texNames[6] = name;
        } else if (key == "textureBack") {
            texNames[7] = name;
        } else if (key == "textureLeft") {
            texNames[8] = name;
        } else if (key == "textureRight") {
            texNames[9] = name;
        }
    });

    // Load all layers
    auto f = makeFunctor([&](Sender, const nString& key, keg::Node value) {
        BlockTextureNames tNames = {};
        // So we can prioritize.
        nString textureNames[10];
        texNames = textureNames;

        blockName = &key;
        context.reader.forAllInMap(value, &valParseFunctor);

        // Set textures based on names
        if (texNames[0].size()) {
            for (int i = 0; i < 6; i++) {
                tNames.names[i] = texNames[0];
            }
        }
        if (texNames[1].size()) {
            tNames.names[(int)vvox::Cardinal::X_NEG] = texNames[1];
            tNames.names[(int)vvox::Cardinal::X_POS] = texNames[1];
        }
        if (texNames[2].size()) {
            tNames.names[(int)vvox::Cardinal::Y_NEG] = texNames[2];
            tNames.names[(int)vvox::Cardinal::Y_POS] = texNames[2];
        }
        if (texNames[3].size()) {
            tNames.names[(int)vvox::Cardinal::Z_NEG] = texNames[3];
            tNames.names[(int)vvox::Cardinal::Z_POS] = texNames[3];
        }
        if (texNames[4].size()) {
            tNames.names[(int)vvox::Cardinal::Y_POS] = texNames[4];
        }
        if (texNames[5].size()) {
            tNames.names[(int)vvox::Cardinal::Y_NEG] = texNames[5];
        }
        if (texNames[6].size()) {
            tNames.names[(int)vvox::Cardinal::Z_POS] = texNames[6];
        }
        if (texNames[7].size()) {
            tNames.names[(int)vvox::Cardinal::Z_NEG] = texNames[7];
        }
        if (texNames[8].size()) {
            tNames.names[(int)vvox::Cardinal::X_NEG] = texNames[8];
        }
        if (texNames[9].size()) {
            tNames.names[(int)vvox::Cardinal::X_POS] = texNames[9];
        }

        // Set mappings
        m_blockMappings[key] = tNames;
    });
    context.reader.forAllInMap(node, &f);
    context.reader.dispose();

    return true;
}

bool BlockTextureLoader::loadLayer(BlockTextureLayer& layer) {
    AtlasTextureDescription desc = m_texturePack->findLayer(layer.path);
    // Check if its already been loaded
    if (desc.size.x != 0) {
        // Already loaded so just use the desc
        // TODO(Ben): Worry about different methods using same file?
        layer.size = desc.size;
        layer.index.layer = desc.index;
    } else {
        vio::Path path;
        if (!m_texturePathResolver->resolvePath(layer.path, path)) return false;
        { // Get pixels for the base texture
            vg::ScopedBitmapResource rs(vg::ImageIO().load(path, vg::ImageIOFormat::RGBA_UI8));
            // Do post processing on the layer
            if (!postProcessLayer(rs, layer)) return false;
        
            layer.index.layer = m_texturePack->addLayer(layer, layer.path, (color4*)rs.bytesUI8v4);
        }
        // Normal map
        if (layer.normalPath.size() && m_texturePathResolver->resolvePath(layer.normalPath, path)) {
            vg::ScopedBitmapResource rs(vg::ImageIO().load(path, vg::ImageIOFormat::RGBA_UI8));
            // Do post processing on the layer
            if (rs.data) {
                layer.index.normal = m_texturePack->addLayer(layer, layer.normalPath, (color4*)rs.bytesUI8v4);
            }
        }
        // disp map
        if (layer.dispPath.size() && m_texturePathResolver->resolvePath(layer.dispPath, path)) {
            vg::ScopedBitmapResource rs(vg::ImageIO().load(path, vg::ImageIOFormat::RGBA_UI8));
            // Do post processing on the layer
            if (rs.data) {
                layer.index.disp = m_texturePack->addLayer(layer, layer.dispPath, (color4*)rs.bytesUI8v4);
            }
        }
    }
    return true;
}

bool BlockTextureLoader::postProcessLayer(vg::ScopedBitmapResource& bitmap, BlockTextureLayer& layer) {

    // ui32 floraRows;
    const ui32& resolution = m_texturePack->getResolution();

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
            layer.size = ui8v2(1);
            DIM_CHECK(width, CONNECTED_WIDTH, bitmap.height, CONNECTED_HEIGHT, CONNECTED);
            break;
        case ConnectedTextureMethods::RANDOM:
            layer.numTiles = bitmap.width / bitmap.height;
            layer.size = ui32v2(1);
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
            layer.size = ui8v2(1);
            DIM_CHECK(width, GRASS_WIDTH, bitmap.height, GRASS_HEIGHT, GRASS);
            break;
        case ConnectedTextureMethods::HORIZONTAL:
            layer.size.x = (ui8)(bitmap.width / resolution);
            layer.size.y = (ui8)(bitmap.height / resolution);
            DIM_CHECK(width, HORIZONTAL_WIDTH, bitmap.height, HORIZONTAL_HEIGHT, HORIZONTAL);
            break;
        case ConnectedTextureMethods::VERTICAL:
            layer.size.x = (ui8)(bitmap.width / resolution);
            layer.size.y = (ui8)(bitmap.height / resolution);
            DIM_CHECK(width, HORIZONTAL_HEIGHT, bitmap.height, HORIZONTAL_WIDTH, VERTICAL);
            break;
        case ConnectedTextureMethods::REPEAT:
            layer.size.x = (ui8)(bitmap.width / resolution);
            layer.size.y = (ui8)(bitmap.height / resolution);
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