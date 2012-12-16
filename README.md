# CCSlidingGridMenu
port Brandon's [SlidingMenuGrid](http://brandonreynolds.com/blog/2011/01/09/cocos2d-sliding-menu-grid/) & add some features

[MIT Licence](http://opensource.org/licenses/mit-license.php), for cocos2dx-2.x

### Usage
    CCArray *items = CCArray::create();
    for (int i = 0; i < 42; i++) {
        auto item = CCMenuItemSprite::create(CCSprite::create("Icon-72.png"), CCSprite::create("Icon-72.png"),
                                             this, menu_selector(HelloWorld::itemCallback));
        items->addObject(item);
    }
    
    CCSlidingGridMenu *gridMenu = CCSlidingGridMenu::create(items, 4, 5, ccp(100, 100), ccp(50, 50), false);
    addChild(gridMenu);
