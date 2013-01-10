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
        CCSprite *sprite = CCSprite::create("Icon-72.png");
        CCMenuItemSprite *item = CCMenuItemSprite::create(sprite, sprite, this, menu_selector(HelloWorld::itemCallback));
        items->addObject(item);
        item->setTag(i);
    }
    
    CCSlidingGridMenu *gridMenu = CCSlidingGridMenu::create(items, 5, 4, CCSizeMake(60, 50), ccp(240, 160), true, 120);
    addChild(gridMenu);
    gridMenu->setShowIndicator(true);
    gridMenu->setIndicatorSprite(CCSprite::create("pages-indicator.png"));
    
    return true;
}

void HelloWorld::itemCallback(CCObject* sender)
{
    auto *item = (CCMenuItem *)sender;
    int index = item->getTag();
    CCLOG("click item %d", index);
}
