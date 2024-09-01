#include "advancedConsole.h"
#include <math.h>

#ifdef __WIN32
#define curs_set(x) 
#endif

const char* gameMap1[] = {
 "#     ##  ##    # ",
 " #    ##  ##     #",
 "#       ##      # ",
 " #     ####      #",
 "#      #  #     # ",
 " #     #  #      #",
};

const char* gameMap0[] = {
",'',,'', ,',  ,', ,'',,'', ,'',,'',  ",
";  ;',,' ; ;  ; ; ;  ;',,' ;  ;',,'  ",
";  ;',,' ; ;  ; ; ;  ;     ;  ;;  ;  ",
";  ;     ; ',,' ; ;  ;,'', ;  ; ;  ; ",
"',,'     ',,'',,' ',,'',,' ',,' ',,' ",
                                     
};

const char* gameMap[] = {
	"##11##1##11##",
	"##11##1##11##",
	"11##11111##11",
	"1####111####1",
	"1#11#111#11#1",
	"1#11#111#11#1"
};

struct tilePartial {
	int id;
	bool hit;
	unsigned char extraData;
};

struct tile {
	int id;
	tilePartial defaultState;
	
	tile() {
		id = 0;
	}
	
	virtual tilePartial getDefaultState() {
		tilePartial partial;
		partial.id = 0;
		partial.hit = true;
		return partial;
	}
	
	virtual void getTexture(tilePartial *tp, float x, float y, wchar_t &ch, color_t &co) {
		ch = L'/';
		co = FWHITE | BBLACK;
	}
	virtual bool isSolid(tilePartial *tp) {
		return false;
	}
	virtual void onHit(tilePartial *tp) {
		tp->hit = true;
	}
};

struct singleHit : public tile {
	singleHit()	{
		id = 1;
	}
	
	void getTexture(tilePartial *tp, float x, float y, wchar_t &ch, color_t &co) override {
		if (!tp->hit) {
			ch = L' ';
			co = FYELLOW | BYELLOW;
		}
	}
	
	bool isSolid(tilePartial *tp) override {
		return !tp->hit;
	}
	
	tilePartial getDefaultState() override {
		tilePartial partial;
		partial.id = 1;
		partial.hit = false;
		return partial;
	}
};

struct coolTexture : public tile {
	coolTexture() {
		id = 2;
	}
	
	bool isSolid(tilePartial *tp) override {
		return !tp->hit;
	}
	
	void getTexture(tilePartial *tp, float x, float y, wchar_t &ch, color_t &co) override {
		if (!tp->hit) {
			ch = L'#';
			co = tp->extraData;
		}
	}
	
	tilePartial getDefaultState() override {
		tilePartial partial;
		partial.id = 2;
		partial.hit = false;
		partial.extraData = color_t((unsigned(rand()) % 7) + 1) | BBLACK;
		return partial;
	}	
};

struct multiHit : public tile {
	multiHit() {
		id = 3;
	}
	
	void getTexture(tilePartial *tp, float x, float y, wchar_t &ch, color_t &co) override {
		if (tp->extraData > 0) {
			ch = L' ';
			co = (tp->extraData + 1) << 4;
		}
	}

	bool isSolid(tilePartial *tp) override {
		return tp->extraData > 0;
	}
	
	void onHit(tilePartial *tp) override {
		if (tp->extraData > 1) {
			tp->extraData--;
			return;
		} else
			if (tp->extraData > 0) {
				tp->extraData--;
				tp->hit = true;
				return;
			}
		return;
	}
	
	tilePartial getDefaultState() override {
		tilePartial partial;
		partial.id = 3;
		partial.hit = false;
		partial.extraData = (rand() % 3) + 2;
		return partial;
	}
};

struct tiles {
	static tile *DefaultTile;
	static tile *SingleHit;
	static tile *CoolTexture;
	static tile *MultiHit;
};

tile *tiles::DefaultTile = new tile;
tile *tiles::SingleHit = new singleHit;
tile *tiles::CoolTexture = new coolTexture;
tile *tiles::MultiHit = new multiHit;

tile *getTile(tilePartial &partial) {
	switch (partial.id) {
		case 0:
			return tiles::DefaultTile;
		case 1:
			return tiles::SingleHit;
		case 2:
			return tiles::CoolTexture;
		case 3:
			return tiles::MultiHit;
	}
	return tiles::DefaultTile;
}

tilePartial *operatingTileMap = nullptr;

char* operatingGameMap = nullptr;

float PI = 3.141592f;
float D_PI = 6.283185f;
int gameMapHeight2 = 14;
int gameMapHeight = 5;
int gameMapWidth2 = 18;
int gameMapWidth = 5;
float ballSpeed = 0.25f;//0.375f;//0.75f;
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
			//if (!operatingTileMap[y * gameMapWidth + x].isSolid())
			//	continue;
				
			//adv::write(x * sizeW + offset, y * sizeH + offset, ' ', FYELLOW | BYELLOW);
			color_t color = FWHITE|BBLACK;// = FYELLOW | BYELLOW;
			wchar_t character = L'/';// = L' ';
			//tilePartial tp = tiles::CoolTexture->getDefaultState();
			//tiles::CoolTexture->getTexture(&tp,0,0,character,color);
			
			getTile(operatingTileMap[y * gameMapWidth + x])->getTexture(&operatingTileMap[y * gameMapWidth + x],0,0,character,color);
			
			adv::fill(
			x * brickSizeW + brickOffset,
			y * brickSizeH + brickOffset,
			x * brickSizeW + brickSizeW + brickOffset,
			y * brickSizeH + brickSizeH + brickOffset,
			character, color);
		}
	}
}

void init() {
		srand(time(NULL));
        gameMapHeight = sizeof(gameMap)/sizeof(gameMap[0]);
        gameMapWidth = 0;
        for (int i = 0; i < gameMapHeight; i++)
	        if (strlen(gameMap[i]) > gameMapWidth)
        		gameMapWidth = strlen(gameMap[i]);

	padWidth = ceilf(float(adv::width) * 0.2f) + 2.0f;//0.2f);
	padHeight = ceilf(float(adv::height) * 0.05f);
	circlePosX = float(adv::width) * 0.5f;
	circlePosY = adv::height - padHeight - 10.0f;
	padPosition = float(adv::width) * 0.5f - (padWidth * 0.5f);
	travelAngle = D_PI * 0.75f;
	brickOffset = 0;//1;
	brickSizeW = floorf((float(adv::width) - (brickOffset * 2)) / gameMapWidth);
	brickOffset = floorf((adv::width - float(brickSizeW * gameMapWidth)) * 0.5f);
	brickSizeH = brickSizeW;
	
	if (operatingTileMap)
		delete [] operatingTileMap;
	if (operatingGameMap)
		delete [] operatingGameMap;

	operatingGameMap = new char[gameMapWidth * gameMapHeight];
	operatingTileMap = new tilePartial[gameMapWidth * gameMapHeight];
		
	for (int y = 0; y < gameMapHeight; y++) {
		for (int x = 0; x < gameMapWidth; x++) {
			operatingGameMap[y * gameMapWidth + x] = gameMap[y][x];
			char c;
			if (((c = operatingGameMap[y * gameMapWidth + x]) == '#') && (c != ' '))
				//if (rand() % 2 == 0)
				//	operatingTileMap[y * gameMapWidth + x] = tiles::SingleHit->getDefaultState();
				//else
					operatingTileMap[y * gameMapWidth + x] = tiles::MultiHit->getDefaultState();
		}
	}
	
	ballSpeed = 0.4f;//adv::height / 55.0f;// / 4.0f;
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
	//adv::fill(0,0,adv::width-1,adv::height-1,' ',BBLUE|FBLACK);
	//adv::fill(brickOffset,brickOffset,adv::width-brickOffset-1,adv::height-brickOffset-1,' ',BBLACK|BBLACK);
	displayGameMap();
	displayPad();
	displayBall();
	//if (brickOffset > 0)
		//adv::fancyBorder(0,0,adv::width-1,adv::height-1,BORDER_DOUBLE,FMAGENTA|BBLACK);	
}

float wrap(float rotation) {
	if (rotation < -D_PI) {
		return wrap(rotation + D_PI);
	} else
		if (rotation > D_PI) {
			return wrap(rotation - D_PI);
		}
	return rotation;
}

void nothing() {
	
}

int wmain() {	
	adv::setThreadState(false);

	#ifdef __linux__
	adv::setDrawingMode(DRAWINGMODE_COMPARE);
	adv::setDoubleWidth(true);
	#endif
	
	init();
	
	int key = 0;
	
	while (NOMOD(key = console::readKeyAsync()) != VK_ESCAPE && key != 'q') {
		adv::clear();
		switch (key) {
			case '>':
			case VK_RIGHT:
			case VK_UP:
				if (padPosition + padWidth + movementMultiplier < adv::width) {
					padPosition += movementMultiplier;
				} else {
					padPosition = adv::width - padWidth;
				}
				break;
			case '<':
			case VK_LEFT:
			case VK_DOWN:
				if (padPosition - movementMultiplier > 0.0f) { 
					padPosition -= movementMultiplier;
				} else {
					padPosition = 0.0f;
				}
				break;
			case '0':
				init();
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
		if (circlePosY > adv::height - padHeight && circlePosX > padPosition && circlePosX < padPosition + padWidth+1) {
			float n = (circlePosX - padPosition) / padWidth;
			//float n = 0.5f;
			n -= 0.5f;
			n *= 0.6f;
			travelAngle = ((-PI * n) + (PI * 0.5f));
		} else
		if (circlePosY > adv::height - padHeight + 2) {
			init();
		}
		
		
		float minX, minY, maxX, maxY;
		minX = brickOffset;
		minY = brickOffset;
		maxX = brickSizeW * gameMapWidth + brickOffset;
		maxY = brickSizeH * gameMapHeight + brickOffset;
		
		char buf[50];
		snprintf(&buf[0], 50, "travelAngle: %f", wrap(travelAngle));
		travelAngle = wrap(travelAngle);
		
		float tolerance = 0.1f;
		for (float step = 0.0f; step <= ballSpeed; step += 0.01f) {//0.01f) {
			float inXfd = cos(travelAngle) * step * 5 + circlePosX;
			float inYfd = -sin(travelAngle) * step * 5 + circlePosY;
			float inXf = cos(travelAngle) * step + circlePosX;
			float inYf = -sin(travelAngle) * step + circlePosY;
			
			//adv::write(inXfd, inYfd, '+', FBLUE | BBLACK);
			//adv::write(inXf, inYf, '+', FYELLOW | BBLACK);
			
			//if (inXf < minX || inXf > maxX - 1 || inYf < minY || inYf > maxY - 1)
			//	continue;
						
			int brickX = ((inXf) - (brickOffset)) / brickSizeW;
			int brickY = ((inYf) - (brickOffset)) / brickSizeH;
			
			if (brickX < 0 || brickX > gameMapWidth - 1 || brickY < 0 || brickY > gameMapHeight - 1)
				continue;
			
			//adv::write(inXfd, inYfd, '+', FBLUE | BRED);			
			//adv::write(inXf, inYf, '+', FYELLOW | BRED);
			
			//if (operatingGameMap[brickY * gameMapWidth + brickX] == ' ')
			if (!getTile(operatingTileMap[brickY * gameMapWidth + brickX])->isSolid(&operatingTileMap[brickY * gameMapWidth + brickX]))
				continue;
			
			
			float minbX, minbY, maxbX, maxbY;			
			minbX = brickSizeW * brickX + brickOffset;
			minbY = brickSizeH * brickY + brickOffset;
			maxbX = brickSizeW * brickX + brickOffset + brickSizeW;
			maxbY = brickSizeH * brickY + brickOffset + brickSizeH;
									
			if (maxbY <= circlePosY - tolerance || circlePosY + tolerance < minbY || maxbX <= circlePosX - tolerance || circlePosX + tolerance < minbX)
				continue;
				
			//float xdist = ((circlePosX - minbX) * (circlePosX - minbX)) + ((circlePosX - maxbX) * (circlePosX - maxbX));
			//float ydist = ((circlePosY - minbY) * (circlePosY - minbY)) + ((circlePosY - maxbY) * (circlePosY - maxbY));
				
			auto square = [](float in) {
				return in * in;
			};
				
			float distl = square(circlePosX - minbX);//square(circlePosX - minbX) + square(circlePosY - minbY);
			float distr = square(circlePosX - maxbX);//square(circlePosX - maxbX) + square(circlePosY - minbY);
			float distt = square(circlePosY - minbY);//square(circlePosX - minbX) + square(circlePosY - maxbY);
			float distb = square(circlePosY - maxbY);//square(circlePosX - maxbX) + square(circlePosY - maxbY);
				
			if ((distl < distb && distl < distt) || (distr < distb && distr < distt)) {
				travelAngle = -PI - travelAngle;
				circlePosY = oldCirclePosY;
				circlePosX = oldCirclePosX;
				
				operatingGameMap[brickY * gameMapWidth + brickX] = ' ';
				getTile(operatingTileMap[brickY * gameMapWidth + brickX])->onHit(&operatingTileMap[brickY * gameMapWidth + brickX]);
				//fprintf(stderr, "vertical wall\r\n");
				break;
			} else {
				//travelAngle = D_PI - travelAngle;
				travelAngle = (PI * 0.5f) - (travelAngle + PI - (PI * 0.5f));
				circlePosY = oldCirclePosY;
				circlePosX = oldCirclePosX;
				//fprintf(stderr, "horizontal wall\r\n");
				operatingGameMap[brickY * gameMapWidth + brickX] = ' ';
				getTile(operatingTileMap[brickY * gameMapWidth + brickX])->onHit(&operatingTileMap[brickY * gameMapWidth + brickX]);
				break;
			}
		}
		
		//Collision against bricks
		
		//Collision against walls
		if (circlePosY < 0)
			//travelAngle = D_PI - travelAngle;
			travelAngle = PI * 0.5f - (travelAngle + PI - PI * 0.5f);
		
		//vertical walls
		if (circlePosX > adv::width || circlePosX < 0)
			//travelAngle = PI - travelAngle;
			travelAngle = -PI - travelAngle;
		
		display();
		//adv::write(0,0,&buf[0], FBLACK | BWHITE);
		adv::waitForReadyFrame();
		adv::draw();
	}

	adv::_advancedConsoleDestruct();
	
	return 0;
}
