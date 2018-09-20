#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
using namespace cocos2d;
class HelloWorld : public cocos2d::Layer
{
public:
    static cocos2d::Scene* createScene();

    virtual bool init();
    // a selector callback
    void menuCloseCallback(cocos2d::Ref* pSender);
	void menuReset(cocos2d::Ref* pSender);
    // implement the "static create()" method manually
	void onKeyPressed(EventKeyboard::KeyCode keyCode, Event* event);
	void onKeyReleased(EventKeyboard::KeyCode keyCode, Event* event);
	void flip(bool t);
	void check_collisions();
	bool drop_missile();
	void add_missile();
	void loadIdle();
	void loadWalking();
	void loadAttack();
	void diff_increment();

    CREATE_FUNC(HelloWorld);
	void update(float) override;

private:
	int score;
	cocos2d::Label* lblscore;
	cocos2d::Sprite* background;
	cocos2d::Sprite* character;
	cocos2d::RepeatForever* idle;//animate
	cocos2d::RepeatForever* walking;//animate
	cocos2d::RepeatForever* walk_left;//moveby
	cocos2d::RepeatForever* walk_right;//moveby
	bool facing_right;
	bool character_attacking;
	bool key_A_Pressed;
	bool key_D_Pressed;
	float missile_odds;
	float missile_speed;
};

#endif // __HELLOWORLD_SCENE_H__
