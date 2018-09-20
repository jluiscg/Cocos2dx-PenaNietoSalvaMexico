#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#define PENA_WIDTH 144
#define PENA_HEIGHT 174
USING_NS_CC;
using namespace cocos2d;
Scene* HelloWorld::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
    auto layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    /////////////////////////////
    // 2. add a menu item with "X" image, which is clicked to quit the program
    //    you may modify it.

    // add a "close" icon to exit the progress. it's an autorelease object
    auto closeItem = MenuItemImage::create(
                                           "CloseNormal.png",
                                           "CloseSelected.png",
                                           CC_CALLBACK_1(HelloWorld::menuCloseCallback, this));
    
    closeItem->setPosition(Vec2(origin.x + visibleSize.width - closeItem->getContentSize().width/2 ,
                                origin.y + closeItem->getContentSize().height/2));

	auto resetbtn = MenuItemImage::create(
										"resteNormal.png",
										"resetSelected.png",
										CC_CALLBACK_1(HelloWorld::menuReset, this));

	resetbtn->setPosition(Vec2(origin.x + visibleSize.width - resetbtn->getContentSize().width / 2 -40,
		origin.y + resetbtn->getContentSize().height / 2 ));

    // create menu, it's an autorelease object
	cocos2d::Vector<MenuItem*> arr;
	arr.pushBack(closeItem);
	arr.pushBack(resetbtn);
    auto menu = Menu::createWithArray(arr);

    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);
	//#########################################################################################
	//mycode
	//setting up the background
	background = Sprite::create("background.png");
	background->setAnchorPoint(Vec2(0, 0));
	background->setPosition(0, 0);
	//init flags and variables
	facing_right = true;
	character_attacking = false;
	srand(time(NULL));
	missile_odds = 9;
	missile_speed = 6 ;
	key_A_Pressed = key_D_Pressed = false;
	//setting up the character
	character = Sprite::create("pena_standing.png");
	character->setPosition(Director::getInstance()->getVisibleSize().width / 2 ,
							Director::getInstance()->getVisibleSize().height / 6-10);
	//start the idle animation
	loadIdle();
	//setting up score label
	score = 0;
	lblscore = Label::createWithTTF(std::to_string(score), "fonts/Marker Felt.ttf", 32);
	lblscore->setAnchorPoint(Vec2(0, 0));
	lblscore->setPosition(0, 680);
	//keyboard listener set up
	auto keyboardListener = EventListenerKeyboard::create();
	keyboardListener->onKeyPressed = CC_CALLBACK_2(HelloWorld::onKeyPressed, this);
	keyboardListener->onKeyReleased = CC_CALLBACK_2(HelloWorld::onKeyReleased, this);
	//adding stuff to the screen
	this->addChild(background);
	this->addChild(character);
	this->addChild(lblscore);
	//adding the listener
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyboardListener, character);
	//start the "update" game loop
	this->scheduleUpdate();
    return true;
}

void HelloWorld::menuCloseCallback(Ref* pSender)
{
    //Close the cocos2d-x game scene and quit the application
    Director::getInstance()->end();

    #if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
    
    /*To navigate back to native iOS screen(if present) without quitting the application  ,do not use Director::getInstance()->end() and exit(0) as given above,instead trigger a custom event created in RootViewController.mm as below*/
    
    //EventCustom customEndEvent("game_scene_close_event");
    //_eventDispatcher->dispatchEvent(&customEndEvent);
}
void HelloWorld::menuReset(Ref* pSender) {
	this->removeChildByTag(5);
	background->removeAllChildren();
	score = 0;
	if (!facing_right)
		flip(false);
	facing_right = true;
	character_attacking = false;
	key_A_Pressed = key_D_Pressed = false;
	missile_odds = 9;
	missile_speed = 6;
	character->setPosition(Director::getInstance()->getVisibleSize().width / 2,
		Director::getInstance()->getVisibleSize().height / 6 - 10);
	loadIdle();
	this->scheduleUpdate();
}

void HelloWorld::update(float delta) {
	//check odds for a new missile
	if (drop_missile()) {
		//log("ading miss\n");
		add_missile();
	}
	//check collisions
	check_collisions();
	//limit character movement in the screen if  its in the limits stop the movement action
	if (character->getPosition().x <= 0) character->stopAction(walk_left);
	else if (character->getPosition().x >= 500) character->stopAction(walk_right);
	//update score label
	lblscore->setString(std::to_string(score));
	//increase difficulty
	//diff_increment(); //difficulty now incrementes every time a missile is destroyed
}
void HelloWorld::diff_increment() {
	//according to the current score increment the
	//odds of missile spawn and the speed of the missiles
	//speed represents the travel time in seconds
	switch (score) {
	case 10:
		missile_odds = 15;
		missile_speed=5;
		break;
	case 20:
		missile_odds = 20;
		missile_speed = 4;
		break;
	case 30:
		missile_odds = 25;
		missile_speed = 3;
		break;
	case 40:
		missile_odds = 30;
		missile_speed = 2;
		break;
	case 50:
		missile_odds = 35;
		
		break;
	}

}
void HelloWorld::check_collisions() {
	//gets the list of all current missiles in the screen
	cocos2d::Vector<cocos2d::Node*> missiles = background->getChildren();
	//then loop through all of them an search for a collision with the player
	for (unsigned int i = 0; i < missiles.size(); i++)
	{
		Sprite* missileSprite = (Sprite*)missiles.at(i);//get a missile
		Rect missileRect = missileSprite->getBoundingBox();//get the bounding box of that missile
		Rect charRect = character->boundingBox();//get the player's bounding box
		//first, check if the missile has reached the floor
		if (missileSprite->getPosition().y <= 100) {
			//if that's the case then
			this->unscheduleUpdate();//stop the game's loop
			background->removeAllChildren();//remove all missiles
			//load and add the death screen
			auto gameover = Sprite::create("death screen.png");
			gameover->setAnchorPoint(Vec2(0,0));
			gameover->setPosition(Vec2(0, 0));
			gameover->setTag(5);//with a tag in order fot the reset method remove the death screen
			this->addChild(gameover);
		}
		//if the missile hasn't reached the floor yet and there is an intersection(collision)
		//between the player and the missile, and the player is currently attacking
		else if (missileRect.intersectsRect(charRect)&&character_attacking)
		{
			//stop the missile's actions (movement, animation and scheduled self remove)
			missileSprite->stopAllActions();
			//replace the sprite with a new sprite explotion in the missile's current position
			//and run a self remove in the missile
			auto pos = missileSprite->getPosition();
			missileSprite->runAction(RemoveSelf::create(true));
			auto expl = Sprite::create("missile Explosion_0.png");
			expl->setPosition(pos);
			this->addChild(expl);
			//schedule the animation, time on screen and self remove of the explotion animation
			cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;
			actions.pushBack(cocos2d::DelayTime::create(0.5));
			actions.pushBack(cocos2d::RemoveSelf::create(true));
			auto sequence = cocos2d::Sequence::create(actions);
			expl->runAction(sequence);
			score++;//increment the score by 1(one)
			missile_odds += 0.3;
			if(missile_speed > 2.5) missile_speed -= 0.07;
		}
	}
}
bool HelloWorld::drop_missile() {
	//the probability of dropping a new missile
	//missile odds / 1000
	return (rand()%1000) < missile_odds;
}
void HelloWorld::add_missile() {
	Sprite* miss = Sprite::create("Tomahawk.png");
	int x = Director::getInstance()->getVisibleSize().width;
	x = 41 + rand() % (x - 82);
	miss->setPosition(x,
		Director::getInstance()->getVisibleSize().height-10);
	background->addChild(miss);
	//animation
	Vector<SpriteFrame*> animFrames;
	animFrames.reserve(4);
	animFrames.pushBack(SpriteFrame::create("animations v2\\toma\\Tomahawk_0.png", Rect(0, 0, 82, 180)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\toma\\Tomahawk_1.png", Rect(0, 0, 82, 180)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\toma\\Tomahawk_2.png", Rect(0, 0, 82, 180)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\toma\\Tomahawk_3.png", Rect(0, 0, 82, 180)));
	// create the animation out of the frames
	Animation* animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
	auto missAni= RepeatForever::create(Animate::create(animation));
	// run a repeat it forever animation
	miss->runAction(missAni);
	//schedule the move and delete sequence of the missile
	cocos2d::Vector<cocos2d::FiniteTimeAction*> actions;
	actions.pushBack(cocos2d::MoveTo::create(missile_speed, cocos2d::Vec2(x, -100)));
	actions.pushBack(cocos2d::RemoveSelf::create(true));
	auto sequence = cocos2d::Sequence::create(actions);
	miss->runAction(sequence);
}
void HelloWorld::flip(bool t) {
	auto flip = cocos2d::FlipX::create(t);
	character->runAction(cocos2d::FlipX::create(t));
}
void HelloWorld::onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event)
{
	if (character_attacking) {
		if (keyCode == EventKeyboard::KeyCode::KEY_A) {
			key_A_Pressed = true;
			facing_right = false;
		}
		else if (keyCode == EventKeyboard::KeyCode::KEY_D) {
			key_D_Pressed = true;
			facing_right = true;
		}
		return;
	}
	character->stopAllActions();
	if (keyCode == EventKeyboard::KeyCode::KEY_A) {
		if (facing_right) {
			facing_right = false;
			character->runAction(cocos2d::FlipX::create(true));
		}
		key_A_Pressed = true;
		walk_left = cocos2d::RepeatForever::create(cocos2d::MoveBy::create(0.2, Vec2(-70, 0)));
		character->runAction(walk_left);
		loadWalking();
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_D) {
		if (!facing_right) {
			facing_right = true;
			character->runAction(cocos2d::FlipX::create(false));
		}
		key_D_Pressed = true;
		walk_right = cocos2d::RepeatForever::create(cocos2d::MoveBy::create(0.2, Vec2(70, 0)));
		character->runAction(walk_right);
		loadWalking();
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE) {
		loadAttack();
		character_attacking = true;
	}

}

void HelloWorld::onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event)
{
	if (keyCode == EventKeyboard::KeyCode::KEY_A) {
		key_A_Pressed = false;
		character->stopAction(walk_left);
		character->stopAction(walking);
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_D) {
		key_D_Pressed = false;
		character->stopAction(walk_right);
		character->stopAction(walking);
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_SPACE) {
		character->stopAllActions();
		character_attacking = false;
		if (key_A_Pressed) {
			walk_left = cocos2d::RepeatForever::create(cocos2d::MoveBy::create(0.2, Vec2(-70, 0)));
			character->runAction(walk_left);
			loadWalking();
		}
		else if (key_D_Pressed) {
			walk_right = cocos2d::RepeatForever::create(cocos2d::MoveBy::create(0.2, Vec2(70, 0)));
			character->runAction(walk_right);
			loadWalking();
		}
		flip(!facing_right);
	}
	else if (keyCode == EventKeyboard::KeyCode::KEY_R) {
		menuReset(nullptr);
		return;
	}
	loadIdle();
}
void HelloWorld::loadAttack() {
	// idle animation
	Vector<SpriteFrame*> animFrames;
	animFrames.reserve(5);
	animFrames.pushBack(SpriteFrame::create("animations v2\\attack\\pena_attack_0.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\attack\\pena_attack_1.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\attack\\pena_attack_2.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\attack\\pena_attack_3.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\attack\\pena_attack_4.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	// create the animation out of the frames
	Animation* animation = Animation::createWithSpriteFrames(animFrames, 0.03f);
	auto attack = RepeatForever::create(Animate::create(animation));
	// run it and repeat it forever
	character->runAction(attack);
}
void HelloWorld::loadIdle() {
	// idle animation
	Vector<SpriteFrame*> animFrames;
	animFrames.reserve(2);
	animFrames.pushBack(SpriteFrame::create("animations v2\\idle\\pena_idle2_0.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\idle\\pena_idle2_1.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	// create the animation out of the frames
	Animation* animation = Animation::createWithSpriteFrames(animFrames, 0.3f);
	idle = RepeatForever::create(Animate::create(animation));
	// run it and repeat it forever
	character->runAction(idle);
}
void HelloWorld::loadWalking() {
	// idle animation
	Vector<SpriteFrame*> animFrames;
	animFrames.reserve(6);
	animFrames.pushBack(SpriteFrame::create("animations v2\\walking\\pena_walking_0.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\walking\\pena_walking_1.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\walking\\pena_walking_2.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\walking\\pena_walking_3.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\walking\\pena_walking_4.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	animFrames.pushBack(SpriteFrame::create("animations v2\\walking\\pena_walking_5.png", Rect(0, 0, PENA_WIDTH, PENA_HEIGHT)));
	// create the animation out of the frames
	Animation* animation = Animation::createWithSpriteFrames(animFrames, 0.1f);
	walking = RepeatForever::create(Animate::create(animation));
	// run it and repeat it forever
	character->runAction(walking);
}