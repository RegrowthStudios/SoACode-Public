#pragma once
class Chunk;
struct LoadData;

// Represents A Chunk Load Task
struct LoadTask {
public:
	// Chunk To Be Loaded
	Chunk* chunk;

	// Loading Information
	LoadData* loadData;

	// Initialize Task With Data
	LoadTask(Chunk *ch = 0, LoadData *ld = 0) {
		chunk = ch;
		loadData = ld;
	}
};