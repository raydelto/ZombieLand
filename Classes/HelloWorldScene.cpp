#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include <iostream>
#include <math.h>
using namespace cocos2d;
using namespace std;

HelloWorld::~HelloWorld() {
	if (_targets) {
		_targets->release();
		_targets = NULL;
	}

	if (_projectiles) {
		_projectiles->release();
		_projectiles = NULL;
	}

}

HelloWorld::HelloWorld() :
		_targets(NULL), _projectiles(NULL){
}

CCScene* HelloWorld::scene() {
	CCScene * scene = NULL;
	do {
		// 'scene' is an autorelease object
		scene = CCScene::create();
		CC_BREAK_IF(!scene);

		// 'layer' is an autorelease object
		HelloWorld *layer = HelloWorld::create();
		CC_BREAK_IF(!layer);

		// add layer as a child to scene
		scene->addChild(layer);
	} while (0);

	// return the scene
	return scene;
}

//For Accelerometer-------------------------------------------------
void HelloWorld::didAccelerate(cocos2d::CCAcceleration* pAccelerationValue) {
	float x = pAccelerationValue->x;                //horizontal acceleration
	float y = pAccelerationValue->y;                //vertical acceleration
	//If you want to move sprite then can use it to change the position..
	CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
	float newX = _player->getPositionX() + (x * 5);
	float newY = _player->getPositionY() + (y * 10);

	if (newX > 5 && newX < visibleSize.width - 7) {
		_player->setPositionX(newX);
	}

	if(newX > visibleSize.width/2){
		CCSize mapSize = _map->getMapSize();
		float mapPosition = (_map->getPositionX() * mapSize.width *0.25) + ( x * 5);
		/*CCLog("MapSize width %f\n", mapSize.width);
		CCLog("Map PositionX %f\n", _map->getPositionX() );
		CCLog("mapPosition  %f\n", mapPosition);*/
		if(_map->getPositionX() > -2560){
			_map -> setPositionX(_map -> getPositionX() +  ( fabs(x) * -5));
		}
	}



	if (newY > 48 && newY <= visibleSize.height - 4) {
		_player->setPositionY(newY);
	}

}

// on "init" you need to initialize your instance
bool HelloWorld::init() {
	bool bRet = false;
	do {
		//////////////////////////////////////////////////////////////////////////
		// super init first
		//////////////////////////////////////////////////////////////////////////

		CC_BREAK_IF(!CCLayerColor::initWithColor(ccc4(193, 250, 143, 255)));
		this->setKeypadEnabled(true);

		CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

		CCSpriteBatchNode *spriteSheet = CCSpriteBatchNode::create("cowboy_sheet.png");
		const int frameWidth = 64;
		const int frameHeight = 64;

		//Move Animation
		_moveAnimation = CCAnimation::create();
		_moveAnimation->setDelayPerUnit(0.05f);
		for (int i = 1; i <= 9; i++) {
			//Add frames of animation
			//CCRectMake to define the width of each frame.
			CCSpriteFrame *frame = CCSpriteFrame::createWithTexture(spriteSheet->getTexture(), CCRectMake(i*(frameWidth), 0, frameWidth, frameHeight));
			if (i == 1) {
				_player = CCSprite::createWithSpriteFrame(frame);
				spriteSheet->addChild(_player, 2);
			}
			_moveAnimation->addSpriteFrame(frame);
			//addFrame(frame);
		}

		//Shoot Animation
		_shootAnimation = CCAnimation::create();
		_shootAnimation->setDelayPerUnit(0.05f);
		for (int i = 11; i <= 13; i++) {
			CCSpriteFrame *frame = CCSpriteFrame::createWithTexture(spriteSheet->getTexture(), CCRectMake(i*(frameWidth), 0, frameWidth, frameHeight));
			_shootAnimation->addSpriteFrame(frame);
		}

		//Running the animation
		CCAnimate *moveAnimate = CCAnimate::create(_moveAnimation);
		_shootAnimate = CCAnimate::create(_shootAnimation);
		_moveAction = CCRepeatForever::create(moveAnimate);
		_shootAction = CCRepeatForever::create(_shootAnimate);
		_player->runAction(_moveAction);

		//_player->runAction( _shootAction);

		//Adding spritesheet to the node.
		this->addChild(spriteSheet, 2);
		//this->addChild(_player, 2);

		_player->setPosition(ccp(origin.x + _player->getContentSize().width/2, origin.y + visibleSize.height/2));
		//this->addChild(_player);

		this->schedule(schedule_selector(HelloWorld::gameLogic), 1.0);

		this->setTouchEnabled(true);
		this->setAccelerometerEnabled(true);

		_targets = new CCArray();
		_projectiles = new CCArray();

		this->schedule(schedule_selector(HelloWorld::updateGame));
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("bg_music.wav", true);

		//Load tile
		_map = CCTMXTiledMap::create("stage4.tmx");
		_map->setAnchorPoint(ccp(0,0));
		_bgLayer = _map->layerNamed("bg");
		this->addChild(_map, 0);

		bRet = true;
	} while (0);

	return bRet;
}

void HelloWorld::menuCloseCallback(CCObject* pSender) {
	// "close" menu item clicked
	CCDirector::sharedDirector()->end();

}

// cpp with cocos2d-x
void HelloWorld::addTarget() {
	CCSprite *target = NULL;

	CCSpriteBatchNode *spriteSheet = CCSpriteBatchNode::create("zombie_sheet.png");
	const int frameWidth = 27;
	const int frameHeight = 46;

	//Move Animation
	CCAnimation* moveAnimation = CCAnimation::create();
	moveAnimation->setDelayPerUnit(0.05f);
	for (int i = 1; i <= 4; i++) {
		CCSpriteFrame *frame = CCSpriteFrame::createWithTexture(spriteSheet->getTexture(), CCRectMake(i*(frameWidth), 0, frameWidth, frameHeight));
		if (i == 1) {
			target = CCSprite::createWithSpriteFrame(frame);
		}
		moveAnimation->addSpriteFrame(frame);
	}

	//Running the animation
	CCAnimate *moveAnimate = CCAnimate::create(moveAnimation);
	CCRepeatForever* moveAction = CCRepeatForever::create(moveAnimate);
	target->runAction(moveAction);

	// Determine where to spawn the target along the Y axis
	CCSize winSize = CCDirector::sharedDirector()->getVisibleSize();
	float minY = target->getContentSize().height / 2;
	float maxY = winSize.height - target->getContentSize().height / 2;
	int rangeY = (int) (maxY - minY);
	// srand( TimGetTicks() );
	int actualY = (rand() % rangeY) + (int) minY;

	// Create the target slightly off-screen along the right edge,
	// and along a random position along the Y axis as calculated
	target->setPosition(ccp(winSize.width + (target->getContentSize().width/2),
			CCDirector::sharedDirector()->getVisibleOrigin().y + actualY));
	this->addChild(target);

	// Determine speed of the target
	int minDuration = (int) 2.0;
	int maxDuration = (int) 4.0;
	int rangeDuration = maxDuration - minDuration;
	// srand( TimGetTicks() );
	int actualDuration = (rand() % rangeDuration) + minDuration;

	// Create the actions
	CCFiniteTimeAction* actionMove = CCMoveTo::create((float) actualDuration, ccp(0 - target->getContentSize().width/2, actualY));
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create(this, callfuncN_selector(HelloWorld::spriteMoveFinished));
	target->runAction(CCSequence::create(actionMove, actionMoveDone, NULL));

	// Add to targets array
	target->setTag(1);
	_targets->addObject(target);
}

void HelloWorld::spriteMoveFinished(CCNode* sender) {
	CCSprite *sprite = (CCSprite *) sender;
	this->removeChild(sprite, true);

	if (sprite->getTag() == 1)  // target
			{
		_targets->removeObject(sprite);


	} else if (sprite->getTag() == 2) // projectile
			{
		_projectiles->removeObject(sprite);
	}
}

void HelloWorld::gameLogic(float dt) {
	this->addTarget();
}


void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event) {
	CCTouch* touch = (CCTouch*) (touches->anyObject());
	CCPoint location = touch->getLocation();

	// Set up initial location of projectile
	CCSize winSize = CCDirector::sharedDirector()->getVisibleSize();
	CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();
	CCSprite *projectile = NULL;
	CCSpriteBatchNode *spriteSheet = CCSpriteBatchNode::create("bullet_sheet.png");
	const int frameWidth = 23;
	const int frameHeight = 22;

	//Move Animation
	CCAnimation* moveAnimation = CCAnimation::create();
	moveAnimation->setDelayPerUnit(0.15f);
	for (int i = 0; i <= 1; i++) {
		//Add frames of animation
		//CCRectMake to define the width of each frame.
		CCSpriteFrame *frame = CCSpriteFrame::createWithTexture(spriteSheet->getTexture(), CCRectMake(i*(frameWidth), 0, frameWidth, frameHeight));
		if (i == 0) {
			projectile = CCSprite::createWithSpriteFrame(frame);
		}
		moveAnimation->addSpriteFrame(frame);
	}

	//Running the animation
	CCAnimate *moveAnimate = CCAnimate::create(moveAnimation);
	CCRepeatForever* moveAction = CCRepeatForever::create(moveAnimate);
	projectile->runAction(moveAction);

	projectile->setPosition(ccp(_player->getPositionX(), _player->getPositionY()));

	float offX = location.x - projectile->getPosition().x;
	float offY = location.y - projectile->getPosition().y;

	// Bail out if we are shooting down or backwards
	if (offX <= 0)
		return;

	// Ok to add now - we've double checked position
	this->addChild(projectile);

	// Determine where we wish to shoot the projectile to
	float realX = origin.x + winSize.width + (projectile->getContentSize().width / 2);
	float ratio = offY / offX;
	float realY = (realX * ratio) + projectile->getPosition().y;
	CCPoint realDest = ccp(realX, realY);

	// Determine the length of how far we're shooting
	float offRealX = realX - projectile->getPosition().x;
	float offRealY = realY - projectile->getPosition().y;
	float length = sqrtf((offRealX * offRealX) + (offRealY * offRealY));
	float velocity = 480 / 1; // 480pixels/1sec
	float realMoveDuration = length / velocity;

	// Move projectile to actual endpoint
	projectile->runAction(
			CCSequence::create(CCMoveTo::create(realMoveDuration, realDest), CCCallFuncN::create(this, callfuncN_selector(HelloWorld::spriteMoveFinished)),
					NULL));

	// Add to projectiles array
	projectile->setTag(2);
	_projectiles->addObject(projectile);

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("shot.wav");
}

void HelloWorld::updateGame(float dt) {
	CCArray *projectilesToDelete = new CCArray;
	CCObject* it = NULL;
	CCObject* jt = NULL;

	// for (it = _projectiles->begin(); it != _projectiles->end(); it++)
	CCARRAY_FOREACH(_projectiles, it) {
		CCSprite *projectile = dynamic_cast<CCSprite*>(it);
		CCRect projectileRect = CCRectMake(
				projectile->getPosition().x - (projectile->getContentSize().width/2),
				projectile->getPosition().y - (projectile->getContentSize().height/2),
				projectile->getContentSize().width,
				projectile->getContentSize().height);

		CCArray* targetsToDelete = new CCArray;

		CCARRAY_FOREACH(_targets, jt) {
			CCSprite *target = dynamic_cast<CCSprite*>(jt);
			CCRect targetRect = CCRectMake(
					target->getPosition().x - (target->getContentSize().width/2),
					target->getPosition().y - (target->getContentSize().height/2),
					target->getContentSize().width,
					target->getContentSize().height);

			if (projectileRect.intersectsRect(targetRect)) {
				//Making explosion with particle system

				CCParticleFire* fire = CCParticleFire::createWithTotalParticles(100);
				fire->setPosition(target->getPosition().x, target->getPosition().y);
				fire->setScaleX(0.5f);
				fire->setScaleY(0.5f);
				fire->setDuration(0.5);
				fire->setAutoRemoveOnFinish(true);
				this->addChild(fire);

				//removing target
				targetsToDelete->addObject(target);
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("uh.wav");
			}
		}

		CCARRAY_FOREACH(targetsToDelete, jt) {
			CCSprite *target = dynamic_cast<CCSprite*>(jt);
			_targets->removeObject(target);
			this->removeChild(target, true);

		}

		if (targetsToDelete->count() > 0) {
			projectilesToDelete->addObject(projectile);
		}
		targetsToDelete->release();
	}

	CCARRAY_FOREACH(projectilesToDelete, it) {
		CCSprite* projectile = dynamic_cast<CCSprite*>(it);
		_projectiles->removeObject(projectile);
		this->removeChild(projectile, true);
	}
	projectilesToDelete->release();
}


void HelloWorld::keyBackClicked() {
    CCLog("\nKeyBack pressed\n");
	CCDirector::sharedDirector()->end();
    exit(0);
}
