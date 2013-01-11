# CCSlidingGridMenu

port Brandon's [SlidingMenuGrid](http://brandonreynolds.com/blog/2011/01/09/cocos2d-sliding-menu-grid/) to cocos2dx-2.x

fix bugs & add features: pages indicator, touch area

[MIT Licence](http://opensource.org/licenses/mit-license.php)

### Usage
    CCArray *items = CCArray::create();
    for (int i = 0; i < 42; i++) {
        CCSprite *sprite = CCSprite::create("Icon-72.png");
        CCMenuItemSprite *item = CCMenuItemSprite::create(sprite, sprite, this, menu_selector(HelloWorld::itemCallback));
        items->addObject(item);
    }
    
    CCSlidingGridMenu *gridMenu = CCSlidingGridMenu::create(items, 5, 4, CCSizeMake(60, 50), true, ccp(240, 160), 120);
    addChild(gridMenu);
    gridMenu->setShowIndicator(true);
    gridMenu->setIndicatorSprite(CCSprite::create("pages-indicator.png"));
    CCEdgeInsets capsInset = {50, 100, 50, 100};
    gridMenu->setTouchAreaWithCapInsets(capsInset);
