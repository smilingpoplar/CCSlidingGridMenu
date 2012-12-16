# CCSlidingGridMenu

port Brandon's [SlidingMenuGrid](http://brandonreynolds.com/blog/2011/01/09/cocos2d-sliding-menu-grid/) & add pages indicator

[MIT Licence](http://opensource.org/licenses/mit-license.php), for cocos2dx-2.x

### Usage
    CCArray *items = CCArray::create();
    for (int i = 0; i < 42; i++) {
        CCMenuItemSprite *item = CCMenuItemSprite::create(CCSprite::create("Icon-72.png"), CCSprite::create("Icon-72.png"),
                                             this, menu_selector(HelloWorld::itemCallback));
        items->addObject(item);
    }
    
    CCSlidingGridMenu *gridMenu = CCSlidingGridMenu::create(items, 5, 4, CCSizeMake(60, 50), ccp(240, 160), true, 120);
    addChild(gridMenu);
