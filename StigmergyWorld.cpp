//  MABE is a product of The Hintze Lab @ MSU
//     for general research information:
//         hintzelab.msu.edu
//     for MABE documentation:
//         github.com/Hintzelab/MABE/wiki
//
//  Copyright (c) 2015 Michigan State University. All rights reserved.
//     to view the full license, visit:
//         github.com/Hintzelab/MABE/wiki/License

#include "StigmergyWorld.h"
//User Parameters
shared_ptr<ParameterLink<int>> StigmergyWorld::evaluationsPerGenerationPL = Parameters::register_parameter("WORLD_STIGMERGY-evaluationsPerGeneration", 1, "Number of times to test each Genome per generation (useful with non-deterministic brains)");
shared_ptr<ParameterLink<int>> StigmergyWorld::lifeTimePL = Parameters::register_parameter("WORLD_STIGMERGY-lifeTime", 1000, "Number of time units an agent lives for");
shared_ptr<ParameterLink<int>> StigmergyWorld::stigmergyBitsPL = Parameters::register_parameter("WORLD_STIGMERGY-stigmergyBits", 1, "Number of bits in stigmergic signal");
shared_ptr<ParameterLink<int>> StigmergyWorld::xDimPL = Parameters::register_parameter("WORLD_STIGMERGY-xDim", 15, "width of world");
shared_ptr<ParameterLink<int>> StigmergyWorld::yDimPL = Parameters::register_parameter("WORLD_STIGMERGY-yDim", 15, "height of world");
shared_ptr<ParameterLink<double>> StigmergyWorld::wallPercentPL = Parameters::register_parameter("WORLD_STIGMERGY-wallPercent", 0.75, "percentage of walls to remove, BETWEEN 0 and 1");


//MABE Parameters
shared_ptr<ParameterLink<string>> StigmergyWorld::groupNamePL = Parameters::register_parameter("WORLD_STIGMERGY_NAMES-groupNameSpace", (string)"root::", "namespace of group to be evaluated");
shared_ptr<ParameterLink<string>> StigmergyWorld::brainNamePL = Parameters::register_parameter("WORLD_STIGMERGY_NAMES-brainNameSpace", (string)"root::", "namespace for parameters used to define brain");

StigmergyWorld::StigmergyWorld(shared_ptr<ParametersTable> _PT):AbstractWorld(_PT)
, lifeTime(lifeTimePL->get(PT))
, stigmergyContentSize(stigmergyBitsPL->get(PT))
, inputSize(
	visionConeArea * visionConeBits + compassSize + stigmergyLocationInputSize +
	stigmergyContentSize + foodInHandSize
	)
, outputSize(
	stigmergyWriteControlSize + stigmergyContentSize +
	stigmergyReadControlSize + movementControlSize
	)
{
	lifeTime = lifeTimePL->get(PT);
	xDim = xDimPL->get(PT);
	if (xDim % 2 == 0){
		xDim++;
	}
	yDim = yDimPL->get(PT);
	if (yDim % 2 == 0){
		yDim++;
	}
	wallPercent = wallPercentPL->get(PT);

	visionConeOffsets = defineVisionCone();
	stigmergyProximitySensorOffsets = defineStigmergySensor();

	// columns to be added to ave file
	popFileColumns.clear();
	popFileColumns.push_back("score");
}

std::vector<std::vector<std::vector<int>>> StigmergyWorld::defineVisionCone(){
	std::vector<std::vector<std::vector<int>>> visionConeOffsets(stigmergyLocationInputSize, std::vector<std::vector<int>>(4, vector<int>(2, 0)));
	// loops over all 4 directions 0-3
	for (int d = 0; d < 4; d++){
		//k tracks the ID of the vision cone location 0-7
		int k = 0;
		//loops over distances away from the agent in the forward direction 1-2
		for (int i = 1; i < 3 ; i++){
			//loops over the width, centered at the agent, conditional on distance away 0-0, (-1)-1, (-2)-2, adjusted to preserve rotational sym.
			if (d == 1){
				for (int j = -i ; j < i+1 ; j++){
					visionConeOffsets[k][d][0] = i; //x
					visionConeOffsets[k][d][1] = j; //y
					printf("%i %i %i %i \n", k, d, i, j);
					k++;
				}
			}else if (d == 3){
				for (int j = i ; j > -i-1 ; j--){
					visionConeOffsets[k][d][0] = -i; //x
					visionConeOffsets[k][d][1] = j; //y
					printf("%i %i %i %i \n", k, d, -i, j);
					k++;
				}
			}else if (d == 2){
				for (int j = i ; j > -i-1 ; j--){
					visionConeOffsets[k][d][0] = j; //x
					visionConeOffsets[k][d][1] = i; //y
					printf("%i %i %i %i \n", k, d, i, j);
					k++;
				}
			}else{ //(d == 0)
				for (int j = -i ; j < i+1 ; j++){
					visionConeOffsets[k][d][0] = j; //x
					visionConeOffsets[k][d][1] = -i; //y
					printf("%i %i %i %i \n", k, d, -i, j);
					k++;
				}
			}
		}
	}
	return visionConeOffsets;
}


std::vector<std::vector<std::vector<int>>> StigmergyWorld::defineStigmergySensor(){
	std::vector<std::vector<std::vector<int>>> stigmergyProximitySensorOffsets(stigmergyLocationInputSize, std::vector<std::vector<int>>(4, vector<int>(2, 0)));
	// loops over all 4 directions 0-3
	for (int d = 0; d < 4; d++){
		//k tracks the ID of the SPS location 0-8
		int k = 0;
		//loops over distances away from the agent in the forward direction 1-(-1)
		for (int i = 1; i > -2 ; i--){
			//loops over the width, centered at the agent,  (-1)-1, adjusted to preserve rotational sym.
			if (d == 1){
				for (int j = -1 ; j < 2 ; j++){
					visionConeOffsets[k][d][0] = i; //x
					visionConeOffsets[k][d][1] = j; //y
					printf("%i %i %i %i \n", k, d, i, j);
					k++;
				}
			}else if (d == 3){
				for (int j = 1 ; j > -2 ; j--){
					visionConeOffsets[k][d][0] = -i; //x
					visionConeOffsets[k][d][1] = j; //y
					printf("%i %i %i %i \n", k, d, -i, j);
					k++;
				}
			}else if (d == 2){
				for (int j = 1 ; j > -2 ; j--){
					visionConeOffsets[k][d][0] = j; //x
					visionConeOffsets[k][d][1] = i; //y
					printf("%i %i %i %i \n", k, d, i, j);
					k++;
				}
			}else{ //(d == 0)
				for (int j = -1 ; j < 2 ; j++){
					visionConeOffsets[k][d][0] = j; //x
					visionConeOffsets[k][d][1] = -i; //y
					printf("%i %i %i %i \n", k, d, -i, j);
					k++;
				}
			}
		}
	}
	return stigmergyProximitySensorOffsets;
}


//Generates a map to evaluate organisms in
void StigmergyWorld::generateMap(){
	//The following algorithm performs a random DFS from the top left corner of the maze
	//to construct the hallways. Note: outer most band is a buffer and should be left solid.

	vector<cell> stack;
	auto allOnes = vector<vector<int>>(xDim, vector<int>(yDim, 1));
	world = allOnes;
	auto current = cell(1,1);
	world[1][1] = 0;
	stack.push_back(current);

	while(not stack.empty()){
		vector<cell> possibleNext;
		for(int dir = 0; dir < 4; dir++){
			if(current.nextIsInBounds(xDim, yDim, dir) and current.nextIsUnvisited(dir, world)){
					possibleNext.push_back(current.next(dir));
			}
		}
		if(not possibleNext.empty()){
			auto target = possibleNext[Random::getIndex(possibleNext.size())];
			stack.push_back(current);
			world[target.x][target.y] = 0;
			world[(current.x + target.x) / 2][(current.y + target.y) / 2] = 0;
			current = target;
		} else {
			if(not stack.empty()){
				current = stack.back();
				stack.pop_back();
			}
		}
	}
	// remove some percentage of walls from the maze to create open spaces
	auto walls = ((xDim - 3)*(yDim - 3)) / 2; //equation found by working out on paper
	auto quota = (int)(walls * wallPercent);

	while(quota > 0){
		auto x = Random::getIndex(xDim -2) + 1;
		auto y = Random::getIndex(yDim -2) + 1;
		if (world[x][y] == 1){
			world[x][y] = 0;
			quota --;
		}
	}
	int foodNum = 1;
	while (foodNum > 0){
		auto x = Random::getIndex(xDim -2) + 1;
		auto y = Random::getIndex(yDim -2) + 1;
		if (world[x][y] == 0){
			world[x][y] = 'F';
			foodNum --;
			foodCount = Random::getIndex(5) + 1;
		}
	}

	int homeNum = 1;
	while (homeNum > 0){
		auto x = Random::getIndex(xDim -2) + 1;
		auto y = Random::getIndex(yDim -2) + 1;
		if (world[x][y] == 0){
			agentX = x;
			agentY = y;
			agentD = Random::getIndex(4);
			world[x][y] = 'H';
			homeNum --;
		}
	}
	//showWorld();

	//Build stigmergyMap
	auto allZeros = vector<vector<int>>(xDim, vector<int>(yDim, 0));
	stigmergyMap = allZeros;
}

void StigmergyWorld::spawnFood(int x, int y){
	world[x][y] = 0;
	int foodNum = 1;
	while (foodNum > 0){
		auto x = Random::getIndex(xDim -2) + 1;
		auto y = Random::getIndex(yDim -2) + 1;
		if (world[x][y] == 0){
			world[x][y] = 'F';
			foodNum --;
			foodCount = Random::getIndex(5) + 1;
		}
	}
}


void StigmergyWorld::showWorld(){
	for(int j=0;j<yDim;j++){
		for(int i=0;i<xDim;i++){
			if (i == agentX && j == agentY){
				switch(agentD){
					case 0:
						printf("%s","^ ");
						break;
					case 1:
						printf("%s","> ");
						break;
					case 2:
						printf("%s"," v");
						break;
					case 3:
						printf("%s"," <");
				}
			}else{
				if (world[i][j] == 0){
					printf("%s","  ");
				}
				else if (world[i][j] == 1){
					printf("%s","██");
				}
				else if (world[i][j] == 'H'){
					printf("%s","HH");
				}
				else if (world[i][j] == 'F'){
					printf("%s","FF");
				}
			}
		}
		printf("\n");
	}
}

void StigmergyWorld::showStigmergyMap(){
	for(int j=0;j<yDim;j++){
		for(int i=0;i<xDim;i++){
			printf("%i",stigmergyMap[i][j]);
		}
		printf("\n");
	}
	printf("------------------\n");
}


void StigmergyWorld::evaluateSolo(shared_ptr<Organism> org, int analyze, int visualize, int debug) {
	auto brain = org->brains[brainNamePL->get(PT)];
	for (int eval = 0; eval < evaluationsPerGenerationPL->get(PT); eval++) {

		// inputs
		std::vector<int> stigmergyContentInput(stigmergyContentSize);
		std::vector<int> foodInHand(foodInHandSize);

		// outputs
		std::vector<int> stigmergyWriteControl(stigmergyWriteControlSize);
		std::vector<int> movementControl(movementControlSize);
		std::vector<int> stigmergyContentOutput(stigmergyContentSize);
		std::vector<int> stigmergyReadControl(stigmergyReadControlSize);

		brain->resetBrain();
		double score = 1.0;
		for (int time = 0; time < StigmergyWorld::lifeTime; time++){
			//reset brain I/O to avoid agents using deactivated nodes as hidden memory.
			brain->resetInputs();
			brain->resetOutputs();

			// set inputs
			int i = 0;
			// VISION CONE ----------------------------------------------------------------------------
			for(int cur = 0; cur < visionConeArea; cur++) {
				auto vcoX = visionConeOffsets[cur][agentD][0];
				auto vcoY = visionConeOffsets[cur][agentD][1];
				//check if vision cone cell is in bounds before filling in the information from that cell
				if ((agentX + vcoX) >= 0 && (agentX + vcoX) < xDim && (agentY + vcoY) >=0 && (agentY + vcoY) < yDim){
					brain->setInput(i, Bit(world[agentX + vcoX][agentY + vcoY] == 0));
					brain->setInput(i+1, Bit(world[agentX + vcoX][agentY + vcoY] == 1));
					brain->setInput(i+2, Bit(world[agentX + vcoX][agentY + vcoY] == 'F'));
					brain->setInput(i+3, Bit(world[agentX + vcoX][agentY + vcoY] == 'H'));
					brain->setInput(i+4, Bit(world[agentX + vcoX][agentY + vcoY] == 'A'));
				}
				else{ // if OOB set to seeing a wall
					brain->setInput(i, 0);
					brain->setInput(i+1, 1);
					brain->setInput(i+2, 0);
					brain->setInput(i+3, 0);
					brain->setInput(i+4, 0);
				}
				i += 5;
			}
			//end VISION   ----------------------------------------------------------------------------

			// compass
			switch(agentD){
				case 0:
				brain->setInput(i, 0);
				brain->setInput(i+1, 0);
				break;
				case 1:
				brain->setInput(i, 1);
				brain->setInput(i+1, 0);
				break;
				case 2:
				brain->setInput(i, 0);
				brain->setInput(i+1, 1);
				break;
				case 3:
				brain->setInput(i, 1);
				brain->setInput(i+1, 1);
			}
			i += 2;
			//  end compass

			//stigmergy proximity sensor ----------------------------------------------------------------------------
			for(int cur = 0; cur < stigmergyLocationInputSize; cur++, i++) {
				auto spsoX = stigmergyProximitySensorOffsets[cur][agentD][0];
				auto spsoY = stigmergyProximitySensorOffsets[cur][agentD][1];
				//check if stigmergy cell is in bounds before filling in the information from that cell
				if ((agentX + spsoX) >= 0 && (agentX + spsoX) < xDim && (agentY + spsoY) >=0 && (agentY + spsoY) < yDim){
					brain->setInput(i, Bit(stigmergyMap[agentX + spsoX][agentY + spsoY] != 0));
				}
				else{ // if OOB set to seeing nothing
					brain->setInput(i, 0);
				}
			}
			//end stigmergy proximity sensor ----------------------------------------------------------------------------

			//stigmergy message sensor
			for( int cur = 0; cur < stigmergyContentSize; cur++, i++){
				if (agentR){
					brain->setInput(i, Bit((stigmergyMap[agentX][agentY] >> cur) % 2));

				}else{
					brain->setInput(i, 0);
				}
			}
			//end stigmergy message sensor

			//food yesno ----------------------------------------------------------------------------
			brain->setInput(i, Bit(agentF));
			i++;
			//end food yesno ----------------------------------------------------------------------------

			// update brain ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			brain->update();

			// read outputs ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
			int o = 0;

			//stigmergy write
			for(int cur = 0; cur < stigmergyWriteControl.size(); ++cur, ++o) {
				stigmergyWriteControl[cur] = Bit(brain->readOutput(o));
			}

			agentW = (bool)stigmergyWriteControl[0];
		
			for(int cur = 0; cur < stigmergyContentOutput.size(); ++cur, ++o) {
				stigmergyContentOutput[cur] = Bit(brain->readOutput(o));
			}

			if (agentW){
				int val = 0;
				for (int cur = 0; cur < stigmergyContentSize; cur++){
					val += stigmergyContentOutput[cur] << cur;
				}
				stigmergyMap[agentX][agentY] = val;
				//showStigmergyMap();
			}
			//end stigmergy write

			//movment begins ----------------------------------------------------------------------------
			for(int cur = 0; cur < movementControl.size(); ++cur, ++o) {
				movementControl[cur] = Bit(brain->readOutput(o));
			}
			auto moveAction = movementControl[0] + 2*movementControl[1];
			switch (moveAction){
				case 0:
				break;
				case 1:
				//turn left
				agentD = (agentD - 1) % 4;
				// takes ABS(agentD) because when a is negative in a%b then (a%b) will also be negative
				if (agentD < 0){
					agentD = agentD * -1;
				}
				break;
				case 2:
				//turn right
				agentD = (agentD + 1) % 4;
				break;
				case 3:
				//move forward
				if (world[agentX + off.x(agentD)][agentY + off.y(agentD)] != 1){
					agentX = agentX + off.x(agentD);
					agentY = agentY + off.y(agentD);
					score = score + (1.0/1000.0);
					// pickup food
					if (world[agentX][agentY] == 'F' and !agentF){
						agentF = true;
						foodCount --;
						if (foodCount < 1){
							spawnFood(agentX, agentY);
						}
						score = score + 1;
					}
					//drop off food
					if (world[agentX][agentY] == 'H' and agentF){
						agentF = false;
						score = score + 0.5;
					}
					//showWorld();
				}
			}
			//movment ends ----------------------------------------------------------------------------
			//read control
			for(int cur = 0; cur < stigmergyReadControl.size(); ++cur, ++o) {
				stigmergyReadControl[cur] = Bit(brain->readOutput(o));
			}
			agentR = (bool)stigmergyReadControl[0];
			//end read control

		}
		//showStigmergyMap();
		org->dataMap.append("score", score);
	}
}
