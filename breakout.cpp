#include "advancedConsole.h"
#include <math.h>

const char* gameMap3[] = {
 "#############           ",
 "#           #           ",
 "#           #           ",
 "#   #       #           ",
 "#   #       #           ",
 "#   #       #           ",
 "#   #########           ",
 "                        ",
 "                        ",



};

const char* gameMap[] = {
 "#     ##  ##    # ",
 " #    ##  ##     #",
 "#       ##      # ",
 " #     ####      #",
 "#      #  #     # ",
 " #     #  #      #",
};

const char* gameMap2[] = {
 "# ###",
 "# #  ",
 "#####",
 "  # #",
 "### #"
};

char* operatingGameMap = nullptr;

float PI = 3.141592f;
float D_PI = 6.283185f;
int gameMapHeight2 = 14;
int gameMapHeight = 5;
int gameMapWidth2 = 18;
int gameMapWidth = 5;
float ballSpeed = 0.25f;//0.75f;
float movementMultiplier = 4.0f;
float travelAngle = 0.0f;
float circlePosX, circlePosY;
float padWidth, padHeight;
float brickOffset;
float brickSizeW, brickSizeH;
float padPosition;

void displayGameMap() {
	for (int x = 0; x < gameMapWidth; x++) {
		for (int y = 0; y < gameMapHeight; y++) {
			if (operatingGameMap[y * gameMapWidth + x] == ' ')
				continue;
				
			//adv::write(x * sizeW + offset, y * sizeH + offset, ' ', FYELLOW | BYELLOW);
			adv::fill(
			x * brickSizeW + brickOffset,
			y * brickSizeH + brickOffset,
			x * brickSizeW + brickSizeW + brickOffset,
			y * brickSizeH + brickSizeH + brickOffset,
			' ', FYELLOW | BYELLOW);
		}
	}
}

void init() {
        gameMapHeight = sizeof(gameMap)/sizeof(gameMap[0]);
        gameMapWidth = 0;
        for (int i = 0; i < gameMapHeight; i++)
	        if (strlen(gameMap[i]) > gameMapWidth)
        		gameMapWidth = strlen(gameMap[i]);

	padWidth = ceilf(float(adv::width) * 0.2f);
	padHeight = ceilf(float(adv::height) * 0.05f);
	circlePosX = float(adv::width) * 0.5f;
	circlePosY = adv::height - padHeight - 2.0f;
	padPosition = float(adv::width) * 0.5f - (padWidth * 0.5f);
	travelAngle = D_PI * 0.75f;
	brickOffset = 0;//1;
	brickSizeW = floorf((float(adv::width) - (brickOffset * 2)) / gameMapWidth);
	brickOffset = floorf((adv::width - float(brickSizeW * gameMapWidth)) * 0.5f);
	brickSizeH = brickSizeW;
	
	if (operatingGameMap)
		delete [] operatingGameMap;

	operatingGameMap = new char[gameMapWidth * gameMapHeight];
		
	for (int y = 0; y < gameMapHeight; y++) {
		for (int x = 0; x < gameMapWidth; x++) {
			operatingGameMap[y * gameMapWidth + x] = gameMap[y][x];
		}
	}
}

void displayGameMap0() {
	for (int y = 0; y < gameMapHeight; y++) {
		for (int x = 0; x < gameMapWidth; x++) {
			if (operatingGameMap[y * gameMapWidth + x] == '#')
				adv::write(x,y,'#',FRED|BBLACK);
		}
	}
}

void displayPad() {
//	adv::fill(padPosition, adv::height, padPosition + padWidth, adv::height - padHeight, L'-', BWHITE | FBLACK);
//	adv::write(padPosition, adv::height - padHeight, L'#',BWHITE|FBLACK);
//	adv::write(padPosition + padWidth, adv::height - padHeight, L'#',BWHITE|FBLACK);
	adv::line(padPosition, adv::height - padHeight, padPosition + padWidth, adv::height - padHeight, L'-', BWHITE | FBLACK);
}

void displayBall() {
	adv::write(circlePosX, circlePosY,L' ',BRED|FBLACK);
}

void display() {
	displayGameMap();
	displayPad();
	displayBall();
	if (brickOffset > 0)
		adv::fancyBorder(0,0,adv::width-1,adv::height-1,BORDER_DOUBLE,FMAGENTA|BBLACK);	
}

int wmain() {
	while (!adv::ready) console::sleep(10);
	
	curs_set(0);
	adv::setDrawingMode(DRAWINGMODE_COMPARE);
	adv::setDoubleWidth(true);
	adv::setThreadState(false);
	
	init();
	
	int key = 0;
	
	while (NOMOD(key = console::readKeyAsync()) != VK_ESCAPE) {
		adv::clear();
		switch (key) {
			case '>':
			case VK_RIGHT:
				if (padPosition + padWidth + movementMultiplier < adv::width) {
					padPosition += movementMultiplier;
				} else {
					padPosition = adv::width - padWidth;
				}
				break;
			case '<':
			case VK_LEFT:
				if (padPosition - movementMultiplier > 0.0f) { 
					padPosition -= movementMultiplier;
				} else {
					padPosition = 0.0f;
				}
				break;
		}


		
		if (padPosition < 0.0f)
			padPosition = 0.0f;
		if (padPosition > adv::width - padWidth)
			padPosition = adv::width - padWidth;
			
		float oldCirclePosX = circlePosX;
		float oldCirclePosY = circlePosY;
		circlePosX += cos(travelAngle) * ballSpeed;
		circlePosY -= sin(travelAngle) * ballSpeed;
		
		//Collision against pad
		if (circlePosY > adv::height - padHeight && circlePosX > padPosition && circlePosX < padPosition + padWidth) {
			float n = (circlePosX - padPosition) / padWidth;
			//float n = 0.5f;
			n -= 0.5f;
			n *= 0.6f;
			travelAngle = ((-PI * n) + (PI * 0.5f));
		} else
		if (circlePosY > adv::height - padHeight + 2) {
			init();
		}
		
		//Collision against bricks
		for (int x = 0; x < gameMapWidth; x++) {
			for (int y = 0; y < gameMapHeight; y++) {
				if (operatingGameMap[y * gameMapWidth + x] == ' ')
					continue;
				
				float minX, minY, maxX, maxY;
				minX = brickSizeW * x + brickOffset;
				minY = brickSizeH * y + brickOffset;
				maxX = brickSizeW * x + brickOffset + brickSizeW;
				maxY = brickSizeH * y + brickOffset + brickSizeH;
				
				if (circlePosX >= minX && circlePosX <= maxX && circlePosY >= minY && circlePosY <= maxY) {
					
					
					
					operatingGameMap[y * gameMapWidth + x] = ' ';
					int xdist = maxX - circlePosX > circlePosX - minX ? maxX - circlePosX : circlePosX - minX;
					int ydist = maxY - circlePosY > circlePosY - minY ? maxY - circlePosY : circlePosY - minY;
					if (xdist < ydist) {
						//fprintf(stderr, "Horizontal collision\r\n");
						travelAngle = PI - travelAngle;
						if (circlePosX - oldCirclePosX > 0.0f)
							circlePosX = minX;
						else
							circlePosX = maxX;
					} else {
						//fprintf(stderr, "Vertical collision\r\n");
						//travelAngle = PI * 0.5f;//PI * 0.25f - travelAngle; //was D_PI
						travelAngle = D_PI - travelAngle;
						if (circlePosY - oldCirclePosY > 0.0f)
							circlePosY = minY;
						else
							circlePosY = maxY;
					}
					//travelAngle += D_PI * 0.75f;
				}
			}
		}
		
		//Collision against walls
		if (circlePosY < 0)
			travelAngle = D_PI - travelAngle;
		
		if (circlePosX > adv::width || circlePosX < 0)
			travelAngle = PI - travelAngle;
		
		display();
		console::sleep(20);
		adv::draw();
	}
	
	return 0;
}
