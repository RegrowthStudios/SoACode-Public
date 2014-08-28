#define _CRT_SECURE_NO_WARNINGS
#include "stdafx.h"
#include "WorldEditor.h"

#include <sys/stat.h>

#include "BlockData.h"
#include "Chunk.h"
#include "ChunkManager.h"
#include "ChunkUpdater.h"
#include "Errors.h"
#include "FileSystem.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Inputs.h"
#include "OpenglManager.h"
#include "Options.h"
#include "Planet.h"
#include "TerrainGenerator.h"
#include "Texture2d.h"
#include "WorldStructs.h"
#include "global.h"
#include "utils.h"

vector <EditorTree *> editorTrees;

Block editorBlockRevert;
Biome *currentEditorBiome = NULL;
NoiseInfo backupNoise;
NoiseInfo *currentEditorNoise = NULL;
bool editorBiomeIsDirty = 0;
int editorBlockID = -1;
int editorBlocksPerChunk = 10;
int editorAltColorActive = 0;
int biomeTemperature = 0;
int biomeRainfall = 0;
bool editorBlockIsDirty = 0;
bool noiseIsActive = 0;
int noiseActive = -1;

int TextureUnitActive = 0;
int TextureIndexSelected = -1, TextureUnitSelected = 0;
int TextureSelectState;

struct EditorBlock{
	int c;
	int blockType;
	Chunk *ch;
};

vector <EditorBlock *> editorBlocks;

void EditorTree::grow()
{
    //check if the tree exists in the world. If it does, we need to ungrow it.
    if (wnodes.size() || lnodes.size()){
        unGrow();
    }
    globalTreeSeed = rand();
    //startChunk->MakeTreeData(td, tt); //might be sending a bad tree..
  
    //startChunk->generateTreeNodes(startc, wnodes, lnodes, td);

    //startChunk->placeTreeNodesAndRecord(wnodes, lnodes);

    needsToGrow = 0;
    startChunk->state = ChunkStates::MESH;
}

void EditorTree::unGrow()
{
    for (int i = lnodes.size() - 1; i >= 0; i--){
        ChunkUpdater::removeBlock(lnodes[i].owner, lnodes[i].c, 0);
        if (lnodes[i].blockType) {
            ChunkUpdater::placeBlock(lnodes[i].owner, lnodes[i].c, lnodes[i].blockType);
        }
    }

    for (int i = wnodes.size()-1; i >= 0; i--){
        ChunkUpdater::removeBlock(wnodes[i].owner, wnodes[i].c, 0);
        if (wnodes[i].blockType) {
            ChunkUpdater::placeBlock(wnodes[i].owner, wnodes[i].c, wnodes[i].blockType);
        }
    }
  

    wnodes.clear();
    lnodes.clear();
}

WorldEditor::WorldEditor(void) : usingChunks(0), _voxelWorld(NULL), _planet(NULL), _editorTree(NULL)
{
	
}

WorldEditor::~WorldEditor(void)
{
	_UI.Destroy();
    if (_editorTree) delete _editorTree;
}

void WorldEditor::initialize(Planet *planet)
{
    _planet = planet;
	if (!_UI.Initialize("./UI/awesomium/", graphicsOptions.screenWidth, graphicsOptions.screenHeight, graphicsOptions.screenWidth, graphicsOptions.screenHeight)){
		exit(1);
	}
	currentUserInterface = &(_UI);

	_chunkCamera.setPosition(glm::dvec3(0.0));
	_chunkCamera.setPitchAngle(0.0f);
	_chunkCamera.setYawAngle(0.0f);
	_chunkCamera.update();

	_voxelWorld = GameManager::voxelWorld;

    FaceData faceData = {};
    _voxelWorld->initialize(glm::dvec3(0, 0, 0), &faceData, _planet, 0, 1);
    _voxelWorld->getChunkManager().generateOnly = true;
    _voxelWorld->getChunkManager().setIsStationary(true);
    GameManager::chunkIOManager->setDisableLoading(true);
    _voxelWorld->beginSession(glm::dvec3(0, 0, 0));
}

//called by main thread
void WorldEditor::update()
{

	if (usingChunks){
		float cameraMoveSpeed = 0.4f * physSpeedFactor;
		float verticalInput = GameManager::inputManager->getAxis(INPUT_VERTICAL);
		float horizontalInput = GameManager::inputManager->getAxis(INPUT_HORIZONTAL);
		_chunkCamera.offsetPosition(verticalInput * _chunkCamera.direction() * cameraMoveSpeed);
		_chunkCamera.offsetPosition(horizontalInput * _chunkCamera.right() * cameraMoveSpeed);


        _voxelWorld->update(_chunkCamera.position(), glm::dvec3(_chunkCamera.direction()));
        if (_editorTree->needsToGrow == 1){
            initializeEditorTree(_editorTree);
            _editorTree->grow();
            _editorTree->needsToGrow--;
        } else if (_editorTree->needsToGrow > 0){
            _editorTree->needsToGrow--;
        }
	}
}

//called by GL thread
void WorldEditor::glUpdate()
{
	_UI.update();
	_chunkCamera.update();

	//if (!chunkManager.GetPositionHeightData((int)currentCamera->focalPoint.x, (int)currentCamera->focalPoint.z, tmpHeightData)){
	//	if (tmpHeightData.height != UNLOADED_HEIGHT){
	//		dcurrBiome = tmpHeightData.biome;
	//		dcurrTemp = tmpHeightData.temperature;
	//		dcurrHumidity = tmpHeightData.rainfall;

	//	}else{
	//		dcurrBiome = NULL;
	//		dcurrTemp = -1;
	//		dcurrHumidity = -1;
	//	}
	//}else{
	//	dcurrBiome = NULL;
	//	dcurrTemp = -1;
	//	dcurrHumidity = -1;
	//}

	//dcurrCh = chunkManager.GetChunk(currentCamera->focalPoint);

	//glm::dmat4 tmpmat(1.0);

	//universal
	if (uiUpdateData.code == -2){
		ReloadTextures();
		uiUpdateData.code = 0;
	}

	switch (EditorState){
		case E_MAIN:
			mainEditorUpdate();
			break;
		case E_TREE_EDITOR:
			treeEditorUpdate();
			break;
		case E_BLOCK_EDITOR:
			blockEditorUpdate();
			break;
		case E_BIOME_EDITOR:
			biomeEditorUpdate();
			break;
		case E_CLIMATE_EDITOR:
			climateEditorUpdate();
			break;
		case E_TERRAIN_EDITOR:
			terrainEditorUpdate();
			break;
	}

	if (EditorState == E_BIOME_EDITOR){
//		chunkManager.update(currentCamera->focalPoint, tmpmat, 0);
	}else{
		//chunkManager.update(glm::dvec3(chunkManager.cX, chunkManager.cY, chunkManager.cZ), tmpmat, 0);
	}
}

void WorldEditor::terrainEditorUpdate()
{
	JSArray args;
	if (uiUpdateData.code != 0){
		switch (uiUpdateData.code){
		case -3:
			args.Push(JSValue(7));
			currentUserInterface->methodHandler.myObject->Invoke(WSLit("ChangeState"), args);
			break;
		case -1: //EXIT
			if (onQuit()) EditorState = E_AEXIT;
			break;
		case 2: //VARIABLE UPDATE
			changeTerrainVariables();
			break;
		case 3: //NOISE REQUEST
			sendNoise(uiUpdateData.id);
			args.Push(JSValue(5));
			currentUserInterface->methodHandler.myObject->Invoke(WSLit("ChangeState"), args);
			break;
		case 8: //CHANGE STATE
			changeState(uiUpdateData.state);
			break;
		}
		uiUpdateData.code = 0;
	}
}

void WorldEditor::climateEditorUpdate()
{
	if (uiUpdateData.code != 0){
		switch (uiUpdateData.code){
		case -1: //EXIT
			if (onQuit()) EditorState = E_AEXIT;
			break;
		case 2: //VARIABLE UPDATE
			changeClimateVariables();
			break;
		case 8: //CHANGE STATE
			changeState(uiUpdateData.state);
			break;
		}
		uiUpdateData.code = 0;
	}
}

void WorldEditor::mainEditorUpdate()
{
	if (uiUpdateData.code != 0){
		switch (uiUpdateData.code){
			case -1: //EXIT
				if (onQuit()) EditorState = E_AEXIT;
				break;
			case 8: //CHANGE STATE
				changeState(uiUpdateData.state);
				break;
		}
		uiUpdateData.code = 0;
	}
}

void WorldEditor::blockEditorUpdate()
{
	int id;
	Chunk *ch;
	if (uiUpdateData.code != 0){
		switch (uiUpdateData.code){
			case -1: //EXIT
                if (onQuit()) EditorState = E_AEXIT;
				break;
			case 2: //LOAD BLOCK
				sendBlockList(1);
				break;
			case 3: //REQUEST BLOCK
				setEditorBlock(uiUpdateData.id, uiUpdateData.state);
				break;
			case 4: //ZOOM TO BLOCK
				int c;
				ch = editorBlocks.back()->ch;
				c = editorBlocks.back()->c;
				//currentCamera->ZoomTo(ch->X + c%chunkWidth + 0.5, ch->Y + c/chunkLayer + 0.5, ch->Z + (c % chunkLayer)/chunkWidth + 0.5, 1.0, glm::dvec3(0), 3.0);
				break;
			case 5:  //CHANGE BLOCK VARIABLES
				changeBlockVariables();
				break;
			case 6: //NEW BLOCK
				sendBlockList(0);
				break;
			case 7: //CHANGE ID
				if (editorBlockID == -1){
					showMessage("Load a block or create a new block first!");
					break;
				}
				sendBlockList(2);
				break;
			case 8: //CHANGE STATE
				if (editorBlockIsDirty){
					id = blockSaveChanges(editorBlockID);
					if (id == 1){ //yes
						fileManager.saveBlocks("../SOA/Data/BlockData.ini");
						editorBlockIsDirty = 0;
					}else if (id == 0){
						Blocks[editorBlockID] = editorBlockRevert;
					}else if (id == -1){
						break;
					}
				}
				changeState(uiUpdateData.state);
				break;
			case 9:  //REGENERATE CHUNKS
				regenerateChunks(0, 1);
				refreshEditorBlocks(Blocks[editorBlockID]);
				break;
			case 10: //CHANGE BLOCK NAME
				if (editorBlockID != -1){
					Blocks[editorBlockID].name = uiUpdateData.variableUpdateData[0]->val;
					editorBlockIsDirty = 1;
				}
				uiUpdateData.variableUpdateData.clear();
				break;
			case 11: //CHANGE TEXTURE
				//if (editorBlockID == -1){
				//	ShowMessage("Load a block or create a new block first!");
				//	break;
				//}
				//GameState = E_SELECT_TEXTURE;
				//TextureUnitActive = 0;
				//TextureIndexSelected = -1;
				//TextureUnitSelected = 0;
				//TextureSelectState = uiUpdateData.state;
				//if (TextureSelectState == 0){
				//	TextureUnitSelected = TextureUnitActive = Blocks[editorBlockID].topTexUnit;
				//	TextureIndexSelected = Blocks[editorBlockID].pyTex;
				//}else if (TextureSelectState == 1){
				//	TextureUnitSelected = TextureUnitActive = Blocks[editorBlockID].sideTexUnit;
				//	TextureIndexSelected = Blocks[editorBlockID].pxTex;
				//}else if (TextureSelectState == 2){
				//	TextureUnitSelected = TextureUnitActive = Blocks[editorBlockID].botTexUnit;
				//	TextureIndexSelected = Blocks[editorBlockID].nyTex;
				//}
				//py = TextureIndexSelected/16;
				//px = TextureIndexSelected%16;
				//DrawWidth = graphicsOptions.screenWidth/2;
				//cellWidth = DrawWidth/16;
				//posX = graphicsOptions.screenWidth / 4;
				//posY = (graphicsOptions.screenHeight - graphicsOptions.screenWidth / 2) / 2;
				////UI.UItexture1.SetPosition(posX + px*cellWidth, (graphicsOptions.screenHeight - posY) - (posY + py*cellWidth));
				break;
			case 12: //Resize altcolors
				if (uiUpdateData.mode == 0){
					if (Blocks[editorBlockID].altColors.size()){
						Blocks[editorBlockID].altColors.pop_back();
						editorBlockIsDirty = 1;
					}
					if (editorAltColorActive > Blocks[editorBlockID].altColors.size()){
						editorAltColorActive = 0;
						refreshEditorBlocks(Blocks[editorBlockID]);
					}
				}else if (uiUpdateData.mode == 1){
					if (Blocks[editorBlockID].altColors.size() < 16){
						Blocks[editorBlockID].altColors.push_back(glm::ivec3(255,255,255));
						editorBlockIsDirty = 1;
					}
				}
				break;
			case 13: //Save Changes
				if (editorBlockIsDirty){
					id = blockSaveChanges(editorBlockID);
					if (id == 1){ //yes
						fileManager.saveBlocks("../SOA/Data/BlockData.ini");
						editorBlockIsDirty = 0;
					}
				}
				break;
		}
		uiUpdateData.code = 0;
	}
}

void WorldEditor::treeEditorUpdate()
{
	int j;
	if (uiUpdateData.code != 0){
		switch (uiUpdateData.code){
			case -1: //EXIT
				if (onQuit()) EditorState = E_AEXIT;
				break;
			case 1: //NEW TREE
                if (!_editorTree->tt) _editorTree->tt = new TreeType;
				loadEditorTree(_editorTree);
				_editorTree->tt->fileName = "";
				_editorTree->dirty = 1;
				break;
			case 2:  //CHANGE TREE VARIABLE
				changeTreeVariables();
				break;
			case 3:  //NEW SEED			
				if (_editorTree->tt){
					srand(SDL_GetTicks());
					globalTreeSeed = rand()%600000 - 300000;
					changeEditorTree(1, _editorTree->tt);
				}
				break;
			case 4:  //LOAD TREE
				loadEditorTree(_editorTree);
				break;
			case 5:  //SAVE TREE
				if (_editorTree && _editorTree->tt){
					if (_editorTree->tt->fileName == ""){
						if (!saveAsEditorTree(_editorTree)){
							showMessage("File Saved Successfully!");
						}
					}else{
						if (!saveEditorTree(_editorTree, _editorTree->tt->fileName)){
							showMessage("File Saved Successfully!");
						}
					}
				}
				break;
			case 6:  //SAVE AS
				if (_editorTree && _editorTree->tt && !saveAsEditorTree(_editorTree)){
					showMessage("File Saved Successfully!");
				}
				break;
			case 7: //CHANGE TREE NAME
				if (_editorTree && _editorTree->tt){
					_editorTree->tt->name = uiUpdateData.variableUpdateData[0]->val;
					_editorTree->dirty = 1;
				}
				uiUpdateData.variableUpdateData.clear();
				break;
			case 8:  //CHANGE STATE
				if (onTreeEditorQuit()){
					changeState(uiUpdateData.state);
				}
				break;
			case 9:  //REGENERATE CHUNKS
				regenerateChunks(0, 1);
				changeEditorTree(0, _editorTree->tt);
				break;
			case 10: //ALT COLOR CHECKBOX INPUT
				if (_editorTree && _editorTree->tt){
					_editorTree->tt->possibleAltLeafFlags.clear();
                    for (size_t i = 0; i < uiUpdateData.args.size(); i++){
						j = uiUpdateData.args[i].ToInteger();
						if (j) _editorTree->tt->possibleAltLeafFlags.push_back(i+1);
					}
					_editorTree->dirty = 1;
					changeEditorTree(0, _editorTree->tt);
				}
				break;
		}
		uiUpdateData.code = 0;
	}
}

void WorldEditor::biomeEditorUpdate()
{
	char buffer[1024];
	NoiseInfo tmp;
	if (uiUpdateData.code != 0){
		switch (uiUpdateData.code){
			case -1: //EXIT
				if (onQuit()) EditorState = E_AEXIT;
				break;
			case 1: //OPEN FILE DIALOG
				if (loadEditorBiome(currentEditorBiome) == 0){
					setEditorBiome(currentEditorBiome);
					biomeTemperature = MIN(MAX((currentEditorBiome->highTemp - currentEditorBiome->lowTemp)/2.0 + currentEditorBiome->lowTemp, 0.0), 255);
					biomeRainfall = MIN(MAX((currentEditorBiome->highRain - currentEditorBiome->lowRain)/2.0 + currentEditorBiome->lowRain, 0.0), 255);
					onClimateChange();
					sendClimate();
				}
				break;
			case 2: //VARIABLE UPDATE
				changeBiomeVariables();
				break;
			case 3: //SET TEMPERATURE
				biomeTemperature = uiUpdateData.id;
				onClimateChange();
				break;
			case 4: //SET RAINFALL
				biomeRainfall = uiUpdateData.id;
				onClimateChange();
				break;
			case 5: //REQUEST NOISE
				sendNoise(uiUpdateData.id);
				break;
			case 6: //CANCEL NOISE
				noiseIsActive = 0;
				*currentEditorNoise = backupNoise;
				appInterfaceChangeState(4);
				_voxelWorld->getPlanet()->flagTerrainForRebuild();
				regenerateChunks(2, 0);
				break;
			case 7: //SAVE
				
				break;
			case 8:	//CHANGE STATE
				if (onBiomeEditorQuit()){
					if (uiUpdateData.state != 4){
				//		editorPlanets[1]->generator.SetHeightModifier(10.0);
				//		*currentEditorBiome = blankBiome;
				//		regenerateChunks(2, 1);
					}
					changeState(uiUpdateData.state);
				}
				break;
			case 9: //SAVE AS

				break;
			case 10: //BACK
				noiseIsActive = 0;
				sendNoiseList();
				appInterfaceChangeState(4);
				break;
			case 11: //CHANGE NAME
				if (currentEditorBiome){
					if (noiseIsActive){
						currentEditorNoise->name = uiUpdateData.variableUpdateData[0]->val;
					}else{
						currentEditorBiome->name = uiUpdateData.variableUpdateData[0]->val;
					}
					editorBiomeIsDirty = 1;
				}
				uiUpdateData.variableUpdateData.clear();
				break;
			case 12: //ADD NOISE
				currentEditorBiome->terrainNoiseList.push_back(NoiseInfo());
				sendNoiseList();
				break;
			case 13: //CHANGE NOISE POSITION
				tmp = currentEditorBiome->terrainNoiseList[uiUpdateData.id];
				if (uiUpdateData.mode == 0){ //DOWN
					if (uiUpdateData.id != 0){
						currentEditorBiome->terrainNoiseList[uiUpdateData.id] = currentEditorBiome->terrainNoiseList[uiUpdateData.id-1];
						currentEditorBiome->terrainNoiseList[uiUpdateData.id-1] = tmp;
						sendNoiseList();
						_voxelWorld->getPlanet()->flagTerrainForRebuild();
						regenerateChunks(2, 0);
					}
				}else{ //UP
					if (uiUpdateData.id != currentEditorBiome->terrainNoiseList.size() - 1){
						currentEditorBiome->terrainNoiseList[uiUpdateData.id] = currentEditorBiome->terrainNoiseList[uiUpdateData.id+1];
						currentEditorBiome->terrainNoiseList[uiUpdateData.id+1] = tmp;
						sendNoiseList();
						_voxelWorld->getPlanet()->flagTerrainForRebuild();
						regenerateChunks(2, 0);
					}
				}
				break;
			case 14: //DELETE NOISE
				sprintf(buffer, "Are you sure you want to delete the noise function %s?", currentEditorBiome->terrainNoiseList[uiUpdateData.id].name.c_str()); 
				if (showYesNoBox(buffer) == 1){
					currentEditorBiome->terrainNoiseList.erase(currentEditorBiome->terrainNoiseList.begin() + uiUpdateData.id);
					sendNoiseList();
					_voxelWorld->getPlanet()->flagTerrainForRebuild();
					regenerateChunks(2, 0);
				}
				break;
			case 15: //HELP
		//		ShowMessage(TerrainFunctionHelps[uiUpdateData.id].c_str());
				break;
			case 16: //SET TYPE
				if (noiseActive){
					currentEditorNoise->type = uiUpdateData.id;
				}
				_voxelWorld->getPlanet()->flagTerrainForRebuild();
				regenerateChunks(2, 0);
				break;
		}
		uiUpdateData.code = 0;
	}

}

void WorldEditor::sendBlockList(int active)
{
	JSArray blockList, args;
	string prompt;
	for (int i = 0; i < LOWWATER; i++){ //right now water is not changeable
		if (active == 1){ //Active
			if (Blocks[i].active){
				blockList.Push(JSValue(i));
				blockList.Push(WSLit(Blocks[i].name.c_str()));
			}
			prompt = "Select a block to load: ";
		}else if (active == 0){ //Non active
			if (!(Blocks[i].active)){
				blockList.Push(JSValue(i));
				blockList.Push(WSLit(""));
			}
			prompt = "Select an unused block slot to load: ";
		}else if (active == 2){ //Both
			if (Blocks[i].active){
				blockList.Push(JSValue(i));
				blockList.Push(WSLit(Blocks[i].name.c_str()));
			}else{
				blockList.Push(JSValue(i));
				blockList.Push(WSLit(""));
			}
			prompt = "Select an block slot to overwrite. Recommend writing to unused slot: ";
		}
	}
	args.Push(JSValue(active));
	args.Push(WSLit(prompt.c_str()));
	args.Push(blockList);
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("GenerateBlockList"), args);
}

#define SET_NOISE_VAR(a, b, c, d, e) variableArray.Push(JSValue(a)); variableArray.Push(JSValue(ToWebString(b))); variableArray.Push(JSValue(c)); variableArray.Push(JSValue(d)); variableArray.Push(JSValue(e)); offset = nit->second.byteOffset; variableArray.Push(JSValue(offset)); if (nit->second.varType == 0){  variableArray.Push(JSValue((int)*(GLuint *)((GLuint)np + offset))); }else if (nit->second.varType == 1){  variableArray.Push(JSValue((float)*(double *)((GLuint)np + offset))); }	

void WorldEditor::sendNoise(int id)
{
	//if (currentNoiseID == -1) return;
//	noiseIsActive = 1;
//	noiseActive = id;
//	NoiseInfo *np;
//	if (EditorState == E_BIOME_EDITOR){
//		if (id == -1){
//			np = &(currentEditorBiome->distributionNoise);
//		}
//		else{
//			np = &(currentEditorBiome->terrainNoiseList[currentNoiseID]);
//		}
//	}
//	else if (EditorState == E_TERRAIN_EDITOR){
//		np = &(GameManager::planet->generator->noiseFunctions[id]);
//	}
//	backupNoise = *np;
//	currentEditorNoise = np;
//	int offset;
//	JSArray variableArray;
//	JSArray args;
//	args.Push(WSLit(np->name.c_str()));
//	int type = np->type;
////	if (type < 0 || type > NumTerrainFunctions) type = 0;
//	args.Push(JSValue(type));
//	if (noiseActive == -1){ //distribution noise
//		auto nit = fileManager.noiseVariableMap.find("Persistence");
//		SET_NOISE_VAR(1, "Persistence", nit->second.min, nit->second.max, nit->second.step)
//			nit = fileManager.noiseVariableMap.find("Frequency");
//		SET_NOISE_VAR(1, "Frequency", nit->second.min, nit->second.max, nit->second.step)
//			nit = fileManager.noiseVariableMap.find("Bound_Low");
//		SET_NOISE_VAR(1, "Occurrence Low Bound", 0.0, 1.0, 0.01)
//			nit = fileManager.noiseVariableMap.find("Bound_High");
//		SET_NOISE_VAR(1, "Transition_Length", 0.0, 1.0, 0.01)
//			nit = fileManager.noiseVariableMap.find("Scale");
//		SET_NOISE_VAR(0, "Scale", 0, 0, 0)
//			nit = fileManager.noiseVariableMap.find("Octaves");
//		SET_NOISE_VAR(1, "Octaves", nit->second.min, nit->second.max, nit->second.step)
//			nit = fileManager.noiseVariableMap.find("Composition");
//		SET_NOISE_VAR(0, "Composition", 0, 0, 0)
//			nit = fileManager.noiseVariableMap.find("Type");
//		SET_NOISE_VAR(0, "Type", 0, 0, 0)
//	}else{
//		for (auto nit = fileManager.noiseVariableMap.begin(); nit != fileManager.noiseVariableMap.end(); nit++){
//			SET_NOISE_VAR(1, nit->first, nit->second.min, nit->second.max, nit->second.step)
//		}
//	}
//	args.Push(variableArray);
//	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetNoiseControls"), args);
}

void WorldEditor::sendNoiseList()
{
	JSArray args, nameArray;
    for (size_t i = 0; i < GameManager::planet->generator->noiseFunctions.size(); i++){
		nameArray.Push(WSLit(GameManager::planet->generator->noiseFunctions[i].name.c_str()));
	}
	args.Push(WSLit("Terrain"));
	args.Push(nameArray);
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetNoiseList"), args);
}

void WorldEditor::setEditorBlock(int id, int state)
{
	JSArray blockList, args, altColors;
	int offset;
	char buffer[1024];
	if (state == 2){ //both active and nonactive blocks
		if (editorBlockID == -1) return;
		if (uiUpdateData.mode == 0){ //swap
			if (Blocks[id].active){
				sprintf(buffer, "Are you sure you want to swap \"%s\" with \"%s\"? This operation can be undone by another swap.", Blocks[editorBlockID].name.c_str(), Blocks[id].name.c_str());
				int id = showYesNoBox(buffer);
				if (id == 0) return;
			}
			if (editorBlockIsDirty){
				sprintf(buffer, "We must save changes to \"%s\". Is this okay?", Blocks[editorBlockID].name.c_str());
				int id = showYesNoBox(buffer);
				if (id == 0) return;
			}
			Block tmp = Blocks[id];
			Blocks[editorBlockID].ID = id;
			Blocks[id] = Blocks[editorBlockID];
			Blocks[editorBlockID] = tmp; //revert it to an empty block
			Blocks[editorBlockID].ID = editorBlockID;
			fileManager.saveBlocks("../SOA/Data/BlockData.ini");
			editorBlockIsDirty = 0;
		}else if (uiUpdateData.mode == 1){ //copy
			if (Blocks[id].active){
				sprintf(buffer, "Are you sure you want to overwrite \"%s\"? This operation cannot be undone.", Blocks[id].name.c_str());
				int id = showYesNoBox(buffer);
				if (id == 0) return;
			}
			if (editorBlockIsDirty){
				sprintf(buffer, "We must save changes to \"%s\". Is this okay?", Blocks[editorBlockID].name.c_str());
				int id = showYesNoBox(buffer);
				if (id == 0) return;
			}
			Blocks[id] = Blocks[editorBlockID];
			Blocks[id].ID = id;
			fileManager.saveBlocks("../SOA/Data/BlockData.ini");
			editorBlockIsDirty = 0;
		}
		editorBlockID = id;
	}else{ //active or nonactive blocks only
		if (editorBlockIsDirty){
			int id = blockSaveChanges(editorBlockID);
			if (id == 1){ //yes
				fileManager.saveBlocks("../SOA/Data/BlockData.ini");
			}else if (id == 0){ //no
				Blocks[editorBlockID] = editorBlockRevert; //revert any changes
			}else if (id == -1){ //cancel
				return;
			}
			editorBlockIsDirty = 0;
		}
		editorBlockRevert = Blocks[id];
		editorBlockID = id;

		refreshEditorBlocks(Blocks[id]);
	}
	
	Block *bp = &(Blocks[id]);
	for (auto bit = blockVariableMap.begin(); bit != blockVariableMap.end(); bit++){
		if (bit->second.editorAccessible){
			offset = bit->second.byteOffset;
			blockList.Push(JSValue(offset));
			if (bit->second.varType == 0){ //int
				blockList.Push(JSValue((int)*(GLuint *)((GLuint)bp + offset)));
			}else if (bit->second.varType == 1){ //float
				blockList.Push(JSValue((float)*(GLfloat *)((GLuint)bp + offset)));
			}else if (bit->second.varType == 2){ //byte
				blockList.Push(JSValue((int)*(GLubyte *)((GLuint)bp + offset)));
			}
		}
	}
    for (size_t i = 0; i < bp->altColors.size(); i++){
		altColors.Push(JSValue(bp->altColors[i].r));
		altColors.Push(JSValue(bp->altColors[i].g));
		altColors.Push(JSValue(bp->altColors[i].b));
	}
	args.Push(WSLit(bp->name.c_str()));
	args.Push(JSValue(bp->ID));
	args.Push(blockList);
	args.Push(altColors);
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetBlock"), args);
	sendBlockTextures();
}

void WorldEditor::setEditorBiome(Biome *biome)
{
	JSArray args, vars;
	int offset;

	args.Push(WSLit(biome->name.c_str()));
	for (auto bit = fileManager.biomeVariableMap.begin(); bit != fileManager.biomeVariableMap.end(); bit++){
		if (bit->second.editorAccessible){
			offset = bit->second.byteOffset;
			vars.Push(JSValue(offset));
			if (bit->second.varType == 0){ //int
				vars.Push(JSValue((int)*(GLuint *)((GLuint)biome + offset)));
			}else if (bit->second.varType == 1){ //float
				vars.Push(JSValue((float)*(GLfloat *)((GLuint)biome + offset)));
			}else if (bit->second.varType == 2){ //ubyte
				vars.Push(JSValue((int)*(GLubyte *)((GLuint)biome + offset)));
			}else if (bit->second.varType == 3){ //ushort
				vars.Push(JSValue((int)*(GLushort *)((GLuint)biome + offset)));
			}
			if (bit->second.byteOffset2 != -1){
				offset = bit->second.byteOffset2;
				vars.Push(JSValue(offset));
				if (bit->second.varType == 0){ //int
					vars.Push(JSValue((int)*(GLuint *)((GLuint)biome + offset)));
				}else if (bit->second.varType == 1){ //float
					vars.Push(JSValue((float)*(GLfloat *)((GLuint)biome + offset)));
				}else if (bit->second.varType == 2){ //ubyte
					vars.Push(JSValue((int)*(GLubyte *)((GLuint)biome + offset)));
				}else if (bit->second.varType == 3){ //ushort
					vars.Push(JSValue((int)*(GLushort *)((GLuint)biome + offset)));
				}
			}
		}
	}
	args.Push(vars);
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetBiome"), args);
	sendIsBase();
	sendNoiseList();
}

void WorldEditor::sendBlockTextures()
{
	/*JSArray args, vals;
	vals.Push(JSValue(Blocks[editorBlockID].pyTex));
	vals.Push(JSValue(Blocks[editorBlockID].topTexUnit));
	vals.Push(JSValue(Blocks[editorBlockID].pxTex));
	vals.Push(JSValue(Blocks[editorBlockID].sideTexUnit));
	vals.Push(JSValue(Blocks[editorBlockID].nyTex));
	vals.Push(JSValue(Blocks[editorBlockID].botTexUnit));
	args.Push(vals);
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetBlockTextures"), args);*/
}

void WorldEditor::refreshEditorBlocks(Block &newBlock)
{
	GLuint strt = SDL_GetTicks();
	Chunk *ch;
	int c;
    for (size_t i = 0; i < editorBlocks.size(); i++){
		ch = editorBlocks[i]->ch;
		c = editorBlocks[i]->c;

//		ch->RemoveBlock(c, chunkManager.setupList, 0);
	}
	GETBLOCK(editorBlockID) = newBlock;
	cout << "TIME1: " << SDL_GetTicks() - strt << endl;

    for (size_t i = 0; i < editorBlocks.size(); i++){
		ch = editorBlocks[i]->ch;
		c = editorBlocks[i]->c;
		editorBlocks[i]->blockType = editorBlockID;
		int id = editorBlockID;
		if (editorAltColorActive){
			int acol = editorAltColorActive;
			SETFLAGS(id, acol);
		}
        ChunkUpdater::placeBlock(ch, c, id);
		if (editorBlocks[i]->ch->numBlocks == 0) editorBlocks[i]->ch->numBlocks = 1;
	}

	cout << "TIME2: " << SDL_GetTicks() - strt << endl;
}

void WorldEditor::appInterfaceChangeState(int newState)
{
	JSArray args;
	args.Push(JSValue(newState));
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("ChangeState"), args);
}

void WorldEditor::changeState(int newState)
{
	cout << "ChangeState: " << newState << endl;
	Awesomium::JSArray args;
	switch (newState){
	case 0:
		EditorState = E_MAIN;
		disableChunks();
		break;
	case 1:
		EditorState = E_TREE_EDITOR;
		enableChunks();
        changeEditorTree(1, _planet->treeTypeVec[0]);
		break;
	case 2:
		EditorState = E_BLOCK_EDITOR;
		enableChunks();
		break;
	case 6:
		EditorState = E_CLIMATE_EDITOR;

		args.Push(WSLit("Climate"));
		args.Push(JSValue(0));
		args.Push(JSValue(GameManager::planet->baseTemperature));
		currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetSliderValue"), args);
		args.Clear();
		args.Push(WSLit("Climate"));
		args.Push(JSValue(1));
		args.Push(JSValue(GameManager::planet->baseRainfall));
		currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetSliderValue"), args);
		disableChunks();
		break;
	case 4: 
		EditorState = E_BIOME_EDITOR;
		enableChunks();
		break;
	case 7: 
		EditorState = E_TERRAIN_EDITOR;
		sendNoiseList();
		sendNoise(2);
		disableChunks();
		break;
	}
	currentUserInterface->changeState(newState);
}

void WorldEditor::changeClimateVariables()
{
    for (size_t i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
		switch (uiUpdateData.variableUpdateData[i]->type){
		case 0: //base temperature
			sscanf(uiUpdateData.variableUpdateData[i]->val.c_str(), "%d", &(GameManager::planet->baseTemperature));
			break;
		case 1: //base humidity
			sscanf(uiUpdateData.variableUpdateData[i]->val.c_str(), "%d", &(GameManager::planet->baseRainfall));
			break;
		}
	}
	uiUpdateData.variableUpdateData.clear();
	glToGame.enqueue(Message(GL_M_REBUILD_TERRAIN, NULL));
}

void WorldEditor::changeTreeVariables()
{
    if (_editorTree == NULL) return;
    TreeType *tt = _editorTree->tt;
	if (tt == NULL) return; //Must have a selected tree

	changeEditorTree(0, tt);
}

void WorldEditor::changeBlockVariables()
{
	if (editorBlockID == -1){
        for (size_t i = 0; i < uiUpdateData.variableUpdateData.size(); i++) delete uiUpdateData.variableUpdateData[i];
		uiUpdateData.variableUpdateData.clear();
		return;
	}
	Block tmpBlock = Blocks[editorBlockID];
	Block *bp = &(tmpBlock);
//	Block *bp = &(Blocks[editorBlockID]);

    for (size_t i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
		if (uiUpdateData.variableUpdateData[i]->type == 1){
			int r, g, b;
			sscanf(uiUpdateData.variableUpdateData[i]->val.c_str(), "%d,%d,%d", &r, &g, &b);
		//	cout << r << " " << g << " " << b << " " << uiUpdateData.variableUpdateData[i]->offset << endl;
			if (uiUpdateData.variableUpdateData[i]->offset == 0){
				editorAltColorActive = 0;
			}else{
				editorAltColorActive = uiUpdateData.variableUpdateData[i]->offset;
				bp->altColors[uiUpdateData.variableUpdateData[i]->offset-1] = glm::ivec3(r, g, b);
			}
		}else{
			if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&bp->explosionRays - (GLuint)bp)){ //integer
				*(GLuint*)((GLuint)bp + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
			}else if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&bp->powerLoss - (GLuint)bp)){ //float
				*(GLfloat*)((GLuint)bp + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atof(uiUpdateData.variableUpdateData[i]->val.c_str());
			}else{ //byte
				*(GLubyte*)((GLuint)bp + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = (GLubyte)atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
			}
		}
		delete uiUpdateData.variableUpdateData[i];
	}
	uiUpdateData.variableUpdateData.clear();
	editorBlockIsDirty = 1;
	refreshEditorBlocks(tmpBlock);
}

void WorldEditor::changeTerrainVariables()
{
	//if (currentNoiseID == -1){
	//	for (int i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
	//		delete uiUpdateData.variableUpdateData[i];
	//	}
	//	uiUpdateData.variableUpdateData.clear();
	//}
	//if (1 || noiseIsActive){
	//	NoiseInfo *np;
	//	if (0 && noiseActive == -1){
	//		np = &(currentEditorBiome->distributionNoise);
	//	}
	//	else{
	//		np = &(GameManager::planet->generator->noiseFunctions[currentNoiseID]);
	//	}
	//	if (np == NULL) return;

	//	for (int i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
	//		if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&np->scale - (GLuint)np)){ //double
	//			*(double*)((GLuint)np + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = (double)atof(uiUpdateData.variableUpdateData[i]->val.c_str());
	//		}
	//		else{ //int
	//			*(GLint*)((GLuint)np + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
	//		}
	//		delete uiUpdateData.variableUpdateData[i];
	//	}
	//	editorBiomeIsDirty = 1;
	//	uiUpdateData.variableUpdateData.clear();
	//}
	//else{
	//	Biome *b = currentEditorBiome;
	//	if (b == NULL) return;

	//	for (int i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
	//		if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&b->isBase - (GLuint)b)){ //integer
	//			*(GLuint*)((GLuint)b + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
	//		}
	//		else if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&b->padding2 - (GLuint)b)){ //short
	//			*(GLushort*)((GLuint)b + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = (GLushort)atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
	//		}
	//		else if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&b->minTerrainMult - (GLuint)b)){ //float
	//			*(GLfloat*)((GLuint)b + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atof(uiUpdateData.variableUpdateData[i]->val.c_str());
	//		}
	//		delete uiUpdateData.variableUpdateData[i];
	//	}
	//	editorBiomeIsDirty = 1;
	//	uiUpdateData.variableUpdateData.clear();
	//}
	//glToGame.enqueue(Message(GL_M_REBUILD_TERRAIN, NULL));
}

void WorldEditor::changeBiomeVariables()
{
	if (noiseIsActive){
		NoiseInfo *np;
		if (noiseActive == -1){
			np = &(currentEditorBiome->distributionNoise);
		}else{
			np = &(currentEditorBiome->terrainNoiseList[noiseActive]);
		}
		if (np == NULL) return;

        for (size_t i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
			if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&np->scale - (GLuint)np)){ //double
				*(double*)((GLuint)np + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = (double)atof(uiUpdateData.variableUpdateData[i]->val.c_str());
			}else{ //int
				*(GLint*)((GLuint)np + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
			}
			delete uiUpdateData.variableUpdateData[i];
		}
		editorBiomeIsDirty = 1;
		uiUpdateData.variableUpdateData.clear();
	}else{
		Biome *b = currentEditorBiome;
		if (b == NULL) return;

        for (size_t i = 0; i < uiUpdateData.variableUpdateData.size(); i++){
			if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&b->isBase - (GLuint)b)){ //integer
				*(GLuint*)((GLuint)b + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
			}else if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&b->padding2 - (GLuint)b)){ //short
				*(GLushort*)((GLuint)b + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = (GLushort)atoi(uiUpdateData.variableUpdateData[i]->val.c_str());
			}else if (uiUpdateData.variableUpdateData[i]->offset <= ((GLuint)&b->minTerrainMult - (GLuint)b)){ //float
				*(GLfloat*)((GLuint)b + (GLuint)uiUpdateData.variableUpdateData[i]->offset) = atof(uiUpdateData.variableUpdateData[i]->val.c_str());
			}
			delete uiUpdateData.variableUpdateData[i];
		}
		editorBiomeIsDirty = 1;
		uiUpdateData.variableUpdateData.clear();
	}
	_voxelWorld->getPlanet()->flagTerrainForRebuild();
	regenerateChunks(2, 0);
}

void WorldEditor::sendIsBase()
{
	//JSArray args;
	//if (currentEditorBiome->isBase){
	//	*(editorPlanets[1]->baseBiomesLookupMap.begin()->second) = *currentEditorBiome;
	//	*(editorPlanets[1]->mainBiomesVector[0]) = blankBiome;
	//	currentEditorBiome = editorPlanets[1]->baseBiomesLookupMap.begin()->second;
	//	args.Push(JSValue(1));
	//}else{
	//	*(editorPlanets[1]->mainBiomesVector[0]) = *currentEditorBiome;
	//	*(editorPlanets[1]->baseBiomesLookupMap.begin()->second) = blankBiome;
	//	currentEditorBiome = editorPlanets[1]->mainBiomesVector[0];
	//	args.Push(JSValue(0));
	//}
	//currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetIsBaseBiome"), args);
}

inline void addVar(JSArray &v, int id, float v1, bool hasSecond = 0, float v2 = 0)
{
    v.Push(JSValue(id));
    v.Push(JSValue(v1));
    v.Push(JSValue(hasSecond));
    if (hasSecond) v.Push(JSValue(v2));
}

void WorldEditor::initializeEditorTree(EditorTree *et)
{
    const deque < deque < deque < ChunkSlot *> > > &chunkList = _voxelWorld->getChunkManager().getChunkList(); //3d deque for chunks
    Chunk *chunk;

    int startX = chunkList.size() / 2;
    int startY = 0;
    int startZ = chunkList[0].size() / 2;
    int blockID;

    for (startY = 0; (startY <= chunkList[0][0].size()) && (et->startChunk == NULL); startY++){
        cout << "X Y Z: " << startX << " " << startY << " " << startZ << endl;
        if (chunkList[startX][startZ][startY]){
            chunk = chunkList[startX][startZ][startY]->chunk;

            if (chunk && chunk->isAccessible){
                for (int y = CHUNK_WIDTH - 1; y >= 0; y--){
                    blockID = chunk->getBlockID(y * CHUNK_LAYER + 16 * CHUNK_WIDTH + 16);
                    if (blockID != 0){
                        cout << Blocks[blockID].name << endl;
                        et->startChunk = chunk;
                        et->startc = y * CHUNK_LAYER + 16 * CHUNK_WIDTH + 16;
                        break;
                    }
                }
            }
        }
    }
}

void WorldEditor::changeEditorTree(bool resendData, TreeType *tt)
{
    if (!_editorTree) {
        _editorTree = new EditorTree;
        _editorTree->tt = tt;  
        _editorTree->needsToGrow = 240;
    } else  {
        _editorTree->needsToGrow = 1;
    }


    switch (uiUpdateData.id) {
    case TREE_INI_BRANCHCHANCEBOTTOM:
        sscanf(&(uiUpdateData.str[0]), "%f,%f", &tt->bottomBranchChance.min, &tt->bottomBranchChance.max);
        break;
    case TREE_INI_BRANCHCHANCETOP:
        sscanf(&(uiUpdateData.str[0]), "%f,%f", &tt->topBranchChance.min, &tt->topBranchChance.max);
        break;
    case TREE_INI_BRANCHCHANCECAPMOD:
        sscanf(&(uiUpdateData.str[0]), "%f", &tt->capBranchChanceMod);
        break;
    case TREE_INI_BRANCHDIRECTIONBOTTOM:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->bottomBranchDir);
        break;
    case TREE_INI_BRANCHDIRTOP:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->topBranchDir);
        break;
    case TREE_INI_BRANCHLENGTHBOTTOM:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->bottomBranchLength.min, &tt->bottomBranchLength.max);
        break;
    case TREE_INI_BRANCHWIDTHBOTTOM:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->bottomBranchWidth.min, &tt->bottomBranchWidth.max);
        break;
    case TREE_INI_BRANCHLEAFSHAPE:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->branchLeafShape);
        break;
    case TREE_INI_BRANCHLEAFSIZEMOD:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->branchLeafSizeMod);
        break;
    case TREE_INI_BRANCHLEAFYMOD:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->branchLeafYMod);
        break;
    case TREE_INI_IDCORE:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->idCore);
        break;
    case TREE_INI_DROOPYLEAVESLENGTH:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->droopyLength.min, &tt->droopyLength.min);
        break;
    case TREE_INI_DROOPYLEAVESSLOPE:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->droopyLeavesSlope);
        break;
    case TREE_INI_DROOPYLEAVESDSLOPE:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->droopyLeavesDSlope);
        break;
    case TREE_INI_DROOPYLEAVESACTIVE:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->hasDroopyLeaves);
        break;
    case TREE_INI_HASTHICKCAPBRANCHES:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->hasThickCapBranches);
        break;
    case TREE_INI_MUSHROOMCAPINVERTED:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->isMushroomCapInverted);
        break;
    case TREE_INI_ISSLOPERANDOM:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->isSlopeRandom);
        break;
    case TREE_INI_LEAFCAPSHAPE:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->leafCapShape);
        break;
    case TREE_INI_LEAFCAPSIZE:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->leafCapSize.min, &tt->leafCapSize.min);
        break;
    case TREE_INI_IDLEAVES:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->idLeaves);
        break;
    case TREE_INI_MUSHROOMCAPCURLLENGTH:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->mushroomCapCurlLength);
        break;
    case TREE_INI_MUSHROOMCAPGILLTHICKNESS:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->mushroomCapGillThickness);
        break;
    case TREE_INI_MUSHROOMCAPSTRETCHMOD:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->mushroomCapLengthMod);
        break;
    case TREE_INI_MUSHROOMCAPTHICKNESS:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->mushroomCapThickness);
        break;
    case TREE_INI_IDBARK:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->idOuter);
        break;
    case TREE_INI_IDROOT:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->idRoot);
        break;
    case TREE_INI_IDSPECIALBLOCK:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->idSpecial);
        break;
    case TREE_INI_BRANCHDIRECTIONTOP:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->topBranchDir);
        break;
    case TREE_INI_BRANCHLENGTHTOP:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->topBranchLength.min, &tt->topBranchLength.max);
        break;
    case TREE_INI_BRANCHWIDTHTOP:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->topBranchWidth.min, &tt->topBranchWidth.max);
        break;
    case TREE_INI_TRUNKHEIGHTBASE:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkBaseHeight.min, &tt->trunkBaseHeight.max);
        break;
    case TREE_INI_TRUNKWIDTHBASE:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkBaseWidth.min, &tt->trunkBaseWidth.max);
        break;
    case TREE_INI_TRUNKCHANGEDIRCHANCE:
        sscanf(&(uiUpdateData.str[0]), "%f", &tt->trunkChangeDirChance);
        break;
    case TREE_INI_TRUNKCOREWIDTH:
        sscanf(&(uiUpdateData.str[0]), "%d", &tt->coreWidth);
        break;
    case TREE_INI_TRUNKSLOPEEND:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkEndSlope.min, &tt->trunkEndSlope.max);
        break;
    case TREE_INI_TRUNKHEIGHT:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkHeight.min, &tt->trunkHeight.max);
        break;
    case TREE_INI_TRUNKWIDTHMID:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkMidWidth.min, &tt->trunkMidWidth.max);
        break;
    case TREE_INI_TRUNKWIDTHTOP:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkTopWidth.min, &tt->trunkTopWidth.max);
        break;
    case TREE_INI_TRUNKSLOPESTART:
        sscanf(&(uiUpdateData.str[0]), "%d,%d", &tt->trunkStartSlope.min, &tt->trunkStartSlope.max);
        break;
    case TREE_INI_ROOTDEPTH:
        sscanf(&(uiUpdateData.str[0]), "%f", &tt->rootDepthMult);
        break;
    case TREE_INI_BRANCHSTART:
        sscanf(&(uiUpdateData.str[0]), "%f", &tt->branchStart);
        break;
    }
    
    if (!resendData) return;

    JSArray v;
    JSArray args;

    v.Push(WSLit(tt->name.c_str()));
    v.Push(JSValue(_editorTree->td.ageMod));

    addVar(v, TREE_INI_BRANCHCHANCEBOTTOM, tt->bottomBranchChance.min, 1, tt->bottomBranchChance.max);
    addVar(v, TREE_INI_BRANCHCHANCETOP, tt->topBranchChance.min, 1, tt->topBranchChance.max);
    addVar(v, TREE_INI_BRANCHCHANCECAPMOD, tt->capBranchChanceMod);
    addVar(v, TREE_INI_BRANCHDIRECTIONBOTTOM, tt->bottomBranchDir);
    addVar(v, TREE_INI_BRANCHDIRTOP, tt->topBranchDir);
    addVar(v, TREE_INI_BRANCHLENGTHBOTTOM, tt->bottomBranchLength.min, 1, tt->bottomBranchLength.max);
    addVar(v, TREE_INI_BRANCHWIDTHBOTTOM, tt->bottomBranchWidth.min, 1, tt->bottomBranchWidth.max);
    addVar(v, TREE_INI_BRANCHLEAFSHAPE, tt->branchLeafShape);
    addVar(v, TREE_INI_BRANCHLEAFSIZEMOD, tt->branchLeafSizeMod);
    addVar(v, TREE_INI_BRANCHLEAFYMOD, tt->branchLeafYMod);
    addVar(v, TREE_INI_BRANCHSTART, tt->branchStart);
    addVar(v, TREE_INI_TRUNKCHANGEDIRCHANCE, tt->trunkChangeDirChance);
    addVar(v, TREE_INI_IDCORE, tt->idCore);
    addVar(v, TREE_INI_DROOPYLEAVESLENGTH, tt->droopyLength.min, 1, tt->droopyLength.max);
    addVar(v, TREE_INI_DROOPYLEAVESSLOPE, tt->droopyLeavesSlope);
    addVar(v, TREE_INI_DROOPYLEAVESDSLOPE, tt->droopyLeavesDSlope);
    addVar(v, TREE_INI_DROOPYLEAVESACTIVE, tt->hasDroopyLeaves);
    addVar(v, TREE_INI_HASTHICKCAPBRANCHES, tt->hasThickCapBranches);
    addVar(v, TREE_INI_MUSHROOMCAPINVERTED, tt->isMushroomCapInverted);
    addVar(v, TREE_INI_ISSLOPERANDOM, tt->isSlopeRandom);
    addVar(v, TREE_INI_LEAFCAPSHAPE, tt->leafCapShape);
    addVar(v, TREE_INI_LEAFCAPSIZE, tt->leafCapSize.min, 1, tt->leafCapSize.max);
    addVar(v, TREE_INI_IDLEAVES, tt->idLeaves);
    addVar(v, TREE_INI_MUSHROOMCAPCURLLENGTH, tt->mushroomCapCurlLength);
    addVar(v, TREE_INI_MUSHROOMCAPGILLTHICKNESS, tt->mushroomCapGillThickness);
    addVar(v, TREE_INI_MUSHROOMCAPSTRETCHMOD, tt->mushroomCapLengthMod);
    addVar(v, TREE_INI_MUSHROOMCAPTHICKNESS, tt->mushroomCapThickness);
    addVar(v, TREE_INI_IDBARK, tt->idOuter);
    addVar(v, TREE_INI_IDROOT, tt->idRoot);
    addVar(v, TREE_INI_IDSPECIALBLOCK, tt->idSpecial);
    addVar(v, TREE_INI_BRANCHDIRECTIONTOP, tt->topBranchDir);
    addVar(v, TREE_INI_BRANCHLENGTHTOP, tt->topBranchLength.min, 1, tt->topBranchLength.max);
    addVar(v, TREE_INI_BRANCHWIDTHTOP, tt->topBranchWidth.min, 1, tt->topBranchWidth.max);
    addVar(v, TREE_INI_TRUNKHEIGHTBASE, tt->trunkBaseHeight.min, 1, tt->trunkBaseHeight.max);
    addVar(v, TREE_INI_TRUNKWIDTHBASE, tt->trunkBaseWidth.min, 1, tt->trunkBaseWidth.max);
    addVar(v, TREE_INI_TRUNKCOREWIDTH, tt->coreWidth);
    addVar(v, TREE_INI_TRUNKSLOPEEND, tt->trunkEndSlope.min, 1, tt->trunkEndSlope.max);
    addVar(v, TREE_INI_TRUNKHEIGHT, tt->trunkHeight.min, 1, tt->trunkHeight.max);
    addVar(v, TREE_INI_TRUNKWIDTHMID, tt->trunkMidWidth.min, 1, tt->trunkMidWidth.max);
    addVar(v, TREE_INI_TRUNKWIDTHTOP, tt->trunkTopWidth.min, 1, tt->trunkTopWidth.max);
    addVar(v, TREE_INI_TRUNKSLOPESTART, tt->trunkStartSlope.min, 1, tt->trunkStartSlope.max);
    addVar(v, TREE_INI_ROOTDEPTH, tt->rootDepthMult);

    args.Push(v);
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetTree"), args);
	
}

void WorldEditor::initializeChunkList(int width, int height, int x, int y, int z)
{
	/*int planetRadius = 2000000;
	editorPlanets[0]->Initialize(planetRadius, 0, 0, 0, 0);
	editorPlanets[1]->Initialize(planetRadius, 0, 0, 0, 1);
	editorPlanets[0]->LoadData(0);
	editorPlanets[1]->colorMapTexture = editorPlanets[1]->colorMapTexture;
	editorPlanets[1]->LoadData(1);
	editorPlanets[1]->generator.SetHeightModifier(10.0);
	blankBiome = *(editorPlanets[1]->allBiomesLookupVector[0]);
	currentEditorBiome = new Biome;
	*currentEditorBiome = blankBiome;
	editorPlanets[1]->AddMainBiome(currentEditorBiome);

	tot2 = 0;
	GLuint startTimer = SDL_GetTicks();
	editorPlanets[1]->InitializeLODs(currentCamera->worldPosition);
	printf("     -LODS: %u\n\n", SDL_GetTicks() - startTimer);
	cout << "LOD Triangles:" << tot2 << endl;

	chunkManager.Initialize(x, z, &(currentCamera->faceData), editorPlanets[1]->radius, height, width, editorPlanets[1], 0);

	chunkManager.InitializeChunks();
	chunkManager.LoadAllChunks(1);*/

}

void WorldEditor::queueAllChunksForLoad()
{
//	chunkManager.LoadAllChunks(2);
}

void WorldEditor::regenerateChunks(int loadType, bool clearDrawing)
{
	//chunkManager.clearAllChunks(clearDrawing);
////	int oldY = chunkManager.Y;
//	chunkManager.regenerateHeightMap(loadType);
//	//int diff = chunkManager.Y - oldY;
//	//if (diff != 0) currentCamera->ZoomTo(currentCamera->focalPoint.x, currentCamera->focalPoint.y + diff, currentCamera->focalPoint.z, 1.0, currentCamera->direction, currentCamera->focalLength);
//	chunkManager.loadAllChunks(loadType);
}

void WorldEditor::enableChunks()
{
	if (!usingChunks){
		_chunkCamera.setPosition(glm::dvec3(0.0));
		//DrawLoadingScreen("Initializing chunk manager...");

		_worldFaceData.Set(0, 0, 0, 0);
		_voxelWorld->initialize(_chunkCamera.position(), &(_worldFaceData), GameManager::planet, 1, 1);
		_chunkCamera.setPosition(glm::dvec3(_chunkCamera.position().x, _voxelWorld->getCenterY(), _chunkCamera.position().z));

		_chunkCamera.update();

		//DrawLoadingScreen("Loading Chunks...");
		_voxelWorld->beginSession(_chunkCamera.position());

		usingChunks = 1;
	}
}

void WorldEditor::disableChunks()
{
	if (usingChunks){
		usingChunks = 0;
		_voxelWorld->endSession();
	}
}

//void WorldEditor::GenerateChunks()
//{
//	Chunk *chunk;
//	chunkManager.setupList.clear();
//	chunkManager.drawList.clear();
//
//	if (currentEditorBiome == NULL) currentEditorBiome = GameManager::planet->allBiomesLookupVector[0];
//
//	for (int x = 0; x < rowSize; x++){
//		for (int z = 0; z < rowSize; z++){
//			for (int y = 0; y < colSize; y++){
//				chunk = chunkManager.chunkList[y][z][x];
//				chunk->Clear();
//			}
//		}
//	}
//	//currTerrainGenerator->GenerateHeight(&heightMap, X, 0, Z, heightMap.size(), heightMap.size(), 0, 0);
//	for (unsigned int i = 0; i < chunkManager.heightMap.size(); i++){
//		for (unsigned int j = 0; j < chunkManager.heightMap[i].size(); j++){
//			chunkManager.heightMap[i][j].flags = 0;
//			chunkManager.heightMap[i][j].height = 10;
//			chunkManager.heightMap[i][j].biome = currentEditorBiome; //set to empty biome
//			chunkManager.heightMap[i][j].rainfall = (currentEditorBiome->lowRain + currentEditorBiome->highRain)/2;
//			chunkManager.heightMap[i][j].temperature = (currentEditorBiome->lowTemp + currentEditorBiome->highTemp)/2;
//			chunkManager.heightMap[i][j].sandDepth = 0;
//			chunkManager.heightMap[i][j].snowDepth = 0;
//			chunkManager.heightMap[i][j].tempOff = 0;
//		}
//	}
//
//	int yi;
//	for (yi = 0; yi < colSize; yi++){
//		chunkManager.LoadChunks(yi);
//	}
//
//	for (int x = 0; x < rowSize; x++){
//		for (int z = 0; z < rowSize; z++){
//			for (int y = 0; y < colSize; y++){
//				chunk = chunkManager.chunkList[y][z][x];
//				chunk->state = TREES;
//			}
//		}
//	}
//
//	for (int x = 0; x < rowSize; x++){
//		for (int z = 0; z < rowSize; z++){
//			for (int y = 0; y < colSize; y++){
//				chunk = chunkManager.chunkList[y][z][x];
//				chunk->CheckEdgeBlocks();
//				if (x > 0) { 
//					chunk->left = chunkManager.chunkList[x-1][z][y];
//					chunk->neighbors++;
//				}
//				if (x < rowSize-1){
//					chunk->right = chunkManager.chunkList[x+1][z][y];
//					chunk->neighbors++;
//				}
//				if (z > 0){
//					chunk->back = chunkManager.chunkList[x][z-1][y];
//					chunk->neighbors++;
//				}
//				if (z < rowSize-1){
//					chunk->front = chunkManager.chunkList[x][z+1][y];
//					chunk->neighbors++;
//				}
//				if (y > 0){
//					chunk->top = chunkManager.chunkList[x][z][y-1];
//					chunk->neighbors++;
//				}
//				if (y < colSize-1){
//					chunk->bottom = chunkManager.chunkList[x][z][y+1];
//					chunk->neighbors++;
//				}
//			}
//		}
//	}
//
//	for (int x = 0; x < rowSize; x++){
//		for (int z = 0; z < rowSize; z++){
//			for (int y = 0; y < colSize; y++){
//				chunk = chunkManager.chunkList[y][z][x];
//				if (chunk->GenerateFlora(chunkManager.setupList)){
//					chunk->state = RENDER;
//				}else{
//					chunk->state = TREES;
//				}
//			}
//		}
//	}
//
////	strtTimer = SDL_GetTicks();
//	for (int x = 0; x < rowSize; x++){
//		for (int z = 0; z < rowSize; z++){
//			for (int y = 0; y < colSize; y++){
//				chunkManager.chunkList[y][z][x]->LoadSunlight(chunkManager.setupList);
//				chunkManager.chunkList[y][z][x]->hasLoadedSunlight = 1;
//				chunkManager.chunkList[y][z][x]->state = RENDER;
//			}
//		}
//	}
//
//	RemoveLights(chunkManager.setupList);
//	for (int i = 0; i < chunkManager.setupList.size(); i++){
//		chunkManager.setupList[i]->setupListPtr = NULL;
//		chunkManager.setupList[i]->updateIndex = -1;
//	}
//	chunkManager.setupList.clear();
//	chunkManager.setupList.swap(vector<Chunk *>());
//
//	for (int x = 0; x < rowSize; x++){
//		for (int z = 0; z < rowSize; z++){
//			for (int y = 0; y < colSize; y++){
//				chunk = chunkManager.chunkList[y][z][x];
//
//				if (chunk->neighbors == 6 && chunk->state == RENDER){
//					if (chunk->lightUpdateQueue.size()){
//						chunk->CalculateLight(chunkManager.setupList);
//					}
//					if (chunk->num && chunk->CreateChunkMesh()){
//						chunk->drawIndex = chunkManager.drawList.size();
//						chunkManager.drawList.push_back(chunk);
//						chunk->state = DRAW;
//					}else{
//						chunk->ClearBuffers();
//						chunk->state = INACTIVE;
//					}
//				}else{
//					chunk->AddToSetupList(chunkManager.setupList, RENDER);
//				}
//			}
//		}
//	}
//}

void WorldEditor::initializeEditorTrees()
{
	/*EditorTree *et = new EditorTree;
	editorTrees.push_back(et);
	_editorTree = et;
	Chunk *startCh;
	startCh = chunkManager.chunkList[chunkManager.chunkList.size()/2][chunkManager.chunkList[0].size()/2][0]; 
	int c = chunkSize - chunkLayer/2;
	if (GameManager::planet->treeTypeVec.size() < 2) exit(43);
	TreeType *tt = GameManager::planet->treeTypeVec[1];

	if (!GetSurfaceBlock(&startCh, c, 0)){
		cout << "Invalid tree base!\n";
		int a;
		cin >> a;
	}else{
		et->startChunk = startCh;
		et->startc = c;
		et->td.startc = c;
		et->tt = NULL;
	}*/
}

void WorldEditor::initializeEditorBlocks()
{
	//EditorBlock *eb;
	//Chunk *ch;
	//int c;
	//bool used[chunkLayer];
	//for (int i = 1; i < chunkManager.chunkList.size()-1; i++){
	//	for (int j = 1; j < chunkManager.chunkList.size()-1; j++){

	//		for (int i = 0; i < chunkLayer; i++) used[i] = 0;

	//		for (int n = 0; n < editorBlocksPerChunk; n++){
	//			c = rand()%chunkLayer;
	//			if (used[c] == 0){
	//				ch = chunkManager.chunkList[i][j][0]; 
	//				c = chunkSize - 1 - c;
	//				if (!GetSurfaceBlock(&ch, c, 1)){
	//					cout << "Invalid block base!\n";
	//					int a;
	//					cin >> a;
	//				}else{
	//					eb = new EditorBlock;
	//					eb->ch = ch;
	//					eb->c = c;
	//					eb->blockType = editorBlockID;
	//					editorBlocks.push_back(eb);
	//				}
	//			}
	//		}
	//	}
	//}
	//ch = chunkManager.chunkList[chunkManager.chunkList.size()/2][chunkManager.chunkList[0].size()/2][0];
	//c = chunkSize - chunkLayer/2;
	//GetSurfaceBlock(&ch, c, 1); //get a floating block 
	//for (int i = 0; i < 32; i++){
	//	if (c < chunkSize-chunkLayer){
	//		c += chunkLayer;
	//	}else if (ch->top && ch->top->isAccessible){
	//		c = c + chunkLayer - chunkSize;
	//		ch = ch->top;
	//	}
	//}
	//eb = new EditorBlock;
	//eb->ch = ch;
	//eb->c = c;
	//eb->blockType = editorBlockID;
	//editorBlocks.push_back(eb);
}

void WorldEditor::removeTree(EditorTree *et)
{
	//Chunk *ch;
	//GLuint c;
	//for (int i = et->wnodes.size()-1; i >= 0; i--){

	//	bool occ = GETBLOCK(*(et->wnodes[i].c)).blockLight || GETBLOCK(*(et->wnodes[i].c)).isLight;
	//	*(et->wnodes[i].c) = et->wnodes[i].blockType;
	//	ch = et->wnodes[i].owner;
	//	c = ((GLuint)et->wnodes[i].c - (GLuint)&(ch->data[0]))/sizeof(GLushort);
	//	ch->AddToSetupList(chunkManager.setupList, RENDER);

	//	if (occ){
	//	bool tmp1 = 0, tmp2 = 0;
	//		if (ch->lightData[0][c] == 0){
	//			tmp1 = 1;
	//			ch->lightData[0][c] = 1;
	//		}
	//		if (ch->lightData[1][c] == 0){
	//			tmp2 = 1;
	//			ch->lightData[1][c] = 1;
	//		}
	//		RemoveLights(ch, c, chunkManager.setupList); //calculates the possible area to be changed by this block break
	//		if (tmp1) ch->lightData[0][c] = 0;
	//		if (tmp2) ch->lightData[1][c] = 0;

	//		int y = c / chunkLayer;
	//		int xz = c-y*chunkLayer;
	//		if (ch->sunLight[xz] == y+1){
	//			if (y == chunkWidth-1){
	//				if (ch->top && ch->top->sunLight[xz] == 0){
	//					ch->ExtendSunRay(xz, y, chunkManager.setupList);
	//				}
	//			}else{
	//				ch->ExtendSunRay(xz, y, chunkManager.setupList);
	//			}
	//		}
	//		//calculate light for all chunks visited
	//	}
	//}
	//for (int i = et->lnodes.size()-1; i >= 0; i--){
	//	bool occ = GETBLOCK(*(et->lnodes[i].c)).blockLight || GETBLOCK(*(et->lnodes[i].c)).isLight;
	//	*(et->lnodes[i].c) = et->lnodes[i].blockType;
	//	ch = et->lnodes[i].owner;
	//	c = ((GLuint)et->lnodes[i].c - (GLuint)&(ch->data[0]))/sizeof(GLushort);
	//	ch->AddToSetupList(chunkManager.setupList, RENDER);

	//	if (occ){
	//	bool tmp1 = 0, tmp2 = 0;
	//		if (ch->lightData[0][c] == 0){
	//			tmp1 = 1;
	//			ch->lightData[0][c] = 1;
	//		}
	//		if (ch->lightData[1][c] == 0){
	//			tmp2 = 1;
	//			ch->lightData[1][c] = 1;
	//		}
	//		RemoveLights(ch, c, chunkManager.setupList); //calculates the possible area to be changed by this block break
	//		if (tmp1) ch->lightData[0][c] = 0;
	//		if (tmp2) ch->lightData[1][c] = 0;

	//		int y = c / chunkLayer;
	//		int xz = c-y*chunkLayer;
	//		if (ch->sunLight[xz] == y+1){
	//			if (y == chunkWidth-1){
	//				if (ch->top && ch->top->sunLight[xz] == 0){
	//					ch->ExtendSunRay(xz, y, chunkManager.setupList);
	//				}
	//			}else{
	//				ch->ExtendSunRay(xz, y, chunkManager.setupList);
	//			}
	//		}
	//		//calculate light for all chunks visited
	//	}
	//}
	//UpdateChunkLights(chunkManager.setupList);
}

int WorldEditor::getSurfaceBlock(Chunk **ch, int &c, bool aboveSurface)
{
	while (1){
		if ((*ch)->getBlockID(c) != NONE){
			if (aboveSurface){
				if (c < CHUNK_SIZE-CHUNK_LAYER){
					c += CHUNK_LAYER;
				}else if ((*ch)->top && (*ch)->top->isAccessible){
					c = c + CHUNK_LAYER - CHUNK_SIZE;
					(*ch) = (*ch)->top;
				}
			}
			return 1;
		}

		if (c >= CHUNK_LAYER){
			c -= CHUNK_LAYER;
		}else if ((*ch)->bottom && (*ch)->bottom->isAccessible){
			c = c - CHUNK_LAYER + CHUNK_SIZE;
			(*ch) = (*ch)->bottom;
		}else{
			return 0;
		}
	}
}

GLushort txboxDrawIndices[6] = {0,1,2,2,3,0};
GLfloat txboxUVs[8] = {0, 1, 0, 0, 1, 0, 1, 1};

//void WorldEditor::DrawSelectTexture()
//{
//	DrawFullScreenQuad(glm::vec4(0.0,0.0,0.0,0.7));
//
//	float DrawHeight = screenWidth/2;
//	float DrawWidth = screenWidth/2;
//	float posX = screenWidth/4;
//	float posY = (screenHeight - screenWidth/2)/2;
//
//	DrawImage2D(posX, posY, DrawWidth, DrawHeight, BlankTextureID, screenWidth, screenHeight, glm::vec4(1.0, 0.0, 1.0, 1.0), 1); //magenta
//	DrawImage2D(posX, posY, DrawWidth, DrawHeight, blockPacks[TextureUnitActive].textureID, screenWidth, screenHeight, glm::vec4(1.0), 1);
//
//	if (TextureIndexSelected != -1 && TextureUnitActive == TextureUnitSelected){
//		DrawImage2D(UI.UItexture1, glm::vec4(1.0), screenWidth, screenHeight);
//	}
//
//	if (TextureUnitActive > 0){
//		if (LeftArrowHover){
//			DrawImage2D(UI.ArrowTextureLeft, glm::vec4(1.3, 1.3, 1.3, 1.0), screenWidth, screenHeight);
//		}else{
//			DrawImage2D(UI.ArrowTextureLeft, glm::vec4(1.0), screenWidth, screenHeight);
//		}
//	}
//	if (TextureUnitActive < blockPacks.size()-1){
//		if (RightArrowHover){
//			DrawImage2D(UI.ArrowTextureRight, glm::vec4(1.3, 1.3, 1.3, 1.0), screenWidth, screenHeight);
//		}else{
//			DrawImage2D(UI.ArrowTextureRight, glm::vec4(1.0), screenWidth, screenHeight);
//		}
//	}
//	//DrawImage2D(vertexPos, sizeof(vertexPos), txboxUVs, sizeof(txboxUVs), txboxDrawIndices, sizeof(txboxDrawIndices), blockPacks[0], glm::vec4(1.0), 0, screenWidth, screenHeight);
//}



//void WorldEditor::DrawBiomeLOD(Camera *camera)
//{
//	float cosTheta = (float)cos(sunTheta);
//	float sinTheta = (float)sin(sunTheta);
//	glm::vec3 lightPos = glm::vec3(cosTheta,sinTheta,0);
//
//	float st = MAX(0.0f, sinTheta + 0.36);
//	if (st > 1) st = 1;
//
//	glm::vec3 dirc = (glm::vec3(camera->worldDirection));
//
//	//far znear for maximum LOD z buffer precision
////	double nearClip = MIN((rowSize/2.0-3.0)*32.0*0.7, 100.0) - ((double)chunkManager.loadList.size()/(double)(rowSize*rowSize*colSize))*80.0;
//	//nearClip = MAX(nearClip, closestLodDistance - 1000);  
//
//	glm::mat4 VP = camera->ProjectionMatrix * camera->ViewMatrix;
//
//	float ambVal = st*(0.76f)+.01f;
//	if (ambVal > 1.0f) ambVal = 1.0f;
//	float diffVal = 1.0f - ambVal;
//	float diffr, diffg, diffb;
//
//	if (sinTheta < 0.0f){
//		diffVal += sinTheta*2;
//		if (diffVal < 0.0f) diffVal = 0.0f;
//	}
//
//	int sh = (int)(sinTheta*64.0f);
//	if (sinTheta < 0){
//		sh = -(int)(sinTheta*64.0f);
//	}
//	diffr = (sunColor[sh][0]/255.0f) * diffVal;
//	diffg = (sunColor[sh][1]/255.0f) * diffVal;
//	diffb = (sunColor[sh][2]/255.0f) * diffVal;
//
//	chunkManager.planet->Draw(sunTheta, VP, camera->ViewMatrix, lightPos, camera->worldPosition, st);
//
//	chunkManager.planet->atmosphere.Draw(sunTheta, VP, lightPos, camera->worldPosition);
//}

//returns 0 on success, 1 on fail
int WorldEditor::saveAsEditorTree(EditorTree *et)
{
	string s = fileManager.getSaveFileNameDialog("Test", "../SOA/World/Trees/TreeTypes");
	if (s.length()){
		saveEditorTree(et, s);
	}else{
		return 1;
	}
	return 0;
}

//returns 0 on success
int WorldEditor::saveEditorTree(EditorTree *et, string fname)
{
	if (et->hasSaved == 0 || et->tt->fileName != fname){
		struct stat buf;
		if (stat(fname.c_str(), &buf) != -1){
			char buffer[1024];
			sprintf(buffer, "Would you like to overwrite %s?", fname.c_str());
			int id = showYesNoCancelBox(buffer);
			if (id == -1 || id == 0) return 1;
			et->hasSaved = 1;
		}
	}
	et->dirty = 0;
	et->tt->fileName = fname;
	fileManager.saveTreeData(et->tt);
	return 0;
}
//returns 0 on success, 1 on fail
int WorldEditor::loadEditorTree(EditorTree *et)
{
    
	bool ok = 1;
	string s = fileManager.getFileNameDialog("Test", "Worlds/Aldrin/Trees/TreeTypes");
	if (s.length()){
		if (_editorTree->dirty == 1){  //SAVE CURRENT TREE
			char buffer[1024];
			sprintf(buffer, "Would you like to save changes to %s?", _editorTree->tt->name.c_str());
			int id = showYesNoCancelBox(buffer);
			if (id == 1){
				if (saveEditorTree(_editorTree, _editorTree->tt->fileName)) ok = 0;
			}else if (id == -1){
				ok = 0; //prevents saving of the file
			}
		}
		if (ok){
			if (_editorTree->tt == NULL) _editorTree->tt = new TreeType;
			if (!fileManager.loadTreeType(s, _editorTree->tt)){
				changeEditorTree(1, _editorTree->tt);
				_editorTree->Refresh();
			}else{
				showMessage("Could not load tree file!");
				return 1;
			}
			_editorTree->dirty = 0;
		}
	}else{
		return 1;
	}
	return 0;
}

int WorldEditor::loadEditorBiome(Biome *currEditorBiome)
{
	bool ok = 1;
	string s = fileManager.getFileNameDialog("Test", "World/Biomes");
	if (s.length()){
		if (editorBiomeIsDirty == 1){  //SAVE CURRENT BIOME
			char buffer[1024];
			sprintf(buffer, "Would you like to save changes to %s?", currentEditorBiome->name.c_str());
			int id = showYesNoCancelBox(buffer);
			if (id == 1){
	//			if (SaveEditorBiome(currentEditorBiome, currentEditorBiome->filename)) ok = 0;
			}else if (id == -1){
				ok = 0; //prevents saving of the file
			}
		}
		if (ok){
			currentEditorBiome->possibleFlora.clear();
			currentEditorBiome->possibleTrees.clear();
			if (!fileManager.readBiome(currentEditorBiome, s, GameManager::planet, "World/")){
  
			}else{
				showMessage("Could not load biome file!");
				return 1;
			}
			editorBiomeIsDirty = 0;
		}
	}else{
		return 1;
	}
	return 0;
}

void WorldEditor::onClimateChange()
{
	/*editorPlanets[1]->generator.SetDefaultTempRain(biomeTemperature, biomeRainfall);
	for (int i = 0; i < chunkManager.heightMap.size(); i++){
		for (int j = 0; j < chunkManager.heightMap[i].size(); j++){
			for (int k = 0; k < chunkLayer; k++){
				chunkManager.heightMap[i][j][k].temperature = biomeTemperature;
				chunkManager.heightMap[i][j][k].rainfall = biomeRainfall;
			}
		}
	}
	chunkManager.planet->FlagTerrainForRebuild();
	RegenerateChunks(2, 0);*/
}

void WorldEditor::sendClimate()
{
	JSArray args;
	args.Push(JSValue(biomeTemperature));
	args.Push(JSValue(biomeRainfall));
	currentUserInterface->methodHandler.myObject->Invoke(WSLit("SetClimate"), args);
}

int WorldEditor::onQuit()
{
	int id = showYesNoBox("Are you sure you want to quit?");
	if (id == 0) return 0;
	switch (EditorState){
		case E_TREE_EDITOR:
			return onTreeEditorQuit();
			break;
		case E_BLOCK_EDITOR:
		case E_SELECT_TEXTURE:
			return onBlockEditorQuit();
			break;
		case E_BIOME_EDITOR:
			return onBiomeEditorQuit();
			break;
	}
	return 1;
}

int WorldEditor::onTreeEditorQuit()
{
	//int id;
	//char buffer[2048];
	//if (_editorTree->dirty){
	//	sprintf(buffer, "Would you like to save changes to \"%s\"?", _editorTree->tt->name.c_str());
	//	id = ShowYesNoCancelBox(buffer);
	//	if (id == 1){
	//		if (saveEditorTree(_editorTree, _editorTree->tt->fileName)) return 0;
	//	}else if (id == -1){
	//		return 0;
	//	}
	//}
	return 1;
}

int WorldEditor::onBlockEditorQuit()
{
	if (editorBlockIsDirty){
		int id = blockSaveChanges(editorBlockID);
		if (id == 1){ //yes
			fileManager.saveBlocks("../SOA/Data/BlockData.ini");
		}else if (id == 0){ //no
			Blocks[editorBlockID] = editorBlockRevert; //revert any changes
		}else if (id == -1){ //cancel
			return 0;
		}
		editorBlockIsDirty = 0;
	}
	return 1;
}

int WorldEditor::onBiomeEditorQuit()
{
	return 1;
}

int WorldEditor::blockSaveChanges(int id)
{
	if (id != -1 && Blocks[id].active){
		char buffer[1024];
		sprintf(buffer, "Would you like to save changes to \"%s\"?", Blocks[id].name.c_str());
		return showYesNoCancelBox(buffer);
	}
	return 0;
}

void WorldEditor::injectMouseMove(int x, int y)
{

    _UI.InjectMouseMove(x, y);

}

void WorldEditor::injectMouseWheel(int yMov)
{
    _UI.injectMouseWheel(yMov);
}

//0 = left, 1 = middle, 2 = right
void WorldEditor::injectMouseDown(int mouseButton)
{
	_UI.InjectMouseDown(mouseButton);
}

void WorldEditor::injectMouseUp(int mouseButton)
{
	if (EditorState == E_SELECT_TEXTURE){

	}else{
		_UI.InjectMouseUp(mouseButton);
	}	
}

void WorldEditor::injectKeyboardEvent(const SDL_Event& event)
{
	if (EditorState == E_SELECT_TEXTURE){

	}else{
		_UI.InjectKeyboardEvent(event);
	}
}

