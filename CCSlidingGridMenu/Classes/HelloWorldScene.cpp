#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"
#include "CCSlidingGridMenu.h"

using namespace cocos2d;
using namespace CocosDenshion;

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // 'layer' is an autorelease object
    HelloWorld *layer = HelloWorld::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
    CCArray *items = CCArray::create();
    for (int i = 0; i < 42; i++) {
        CCMenuItemSprite *item = CCMenuItemSprite::create(CCSprite::create("Icon-72.png"),
                                             CCSprite::create("Icon-72.png"),
                                             this,
                                             menu_selector(HelloWorld::itemCallback));
        items->addObject(item);
    }
    
    CCSlidingGridMenu *gridMenu = CCSlidingGridMenu::create(items, 4, 4, CCSizeMake(60, 50), ccp(240, 160), true, 100);
    addChild(gridMenu);
    
    return true;
}

void HelloWorld::itemCallback(CCObject* sender)
{
    auto *item = (CCMenuItem *)sender;
    int index = item->getTag();
    CCLOG("click item %d", index);
}
