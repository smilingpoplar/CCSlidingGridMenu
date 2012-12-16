#include "CCSlidingGridMenu.h"

NS_CC_BEGIN

CCSlidingGridMenu* CCSlidingGridMenu::create(CCArray *items, int cols, int rows, const CCPoint &position, const CCPoint &padding, bool vertical) {
	CCSlidingGridMenu *slidingMenu = new CCSlidingGridMenu();
	if (slidingMenu && slidingMenu->init(items, cols, rows, position, padding, vertical)) {
		slidingMenu->autorelease();
		return slidingMenu;
	}
	CC_SAFE_DELETE(slidingMenu);
	return NULL;
}

bool CCSlidingGridMenu::init(CCArray *items, int cols, int rows, const CCPoint &position, const CCPoint &padding, bool vertical) {
	if(!CCLayer::init()) return false;

	setTouchEnabled(true);
    
    int tag = 0;
    CCObject *itemObject;
    CCMenuItemSprite *item;
    CCARRAY_FOREACH(items, itemObject) {
        item = (CCMenuItemSprite*)itemObject;
        addChild(item, 0, tag);
        tag++;
    }
    
	_state = kCCMenuStateWaiting;
	_selectedItem = NULL;
    _padding = padding;
	_pageCount = 0;
	_currentPage = 0;
	_isMoving = false;
    _isSwipeOnlyOnMenu = false;
	_menuOrigin = position;
	_minMoveDistance = 10;
	_isVerticalPaging = vertical;
    
	_isVerticalPaging ? buildVerticalGrid(cols, rows) : buildHorizontalGrid(cols, rows);
	setPosition(_menuOrigin);
    
	return true;
}

void CCSlidingGridMenu::addChild(CCNode *child, int zOrder, int tag) {
    CCAssert(dynamic_cast<CCMenuItem*>(child) != NULL, "Menu only supports MenuItem objects as children");
    CCLayer::addChild(child, zOrder, tag);
}

void CCSlidingGridMenu::buildHorizontalGrid(int cols, int rows) {
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	int col = 0, row = 0;
	CCObject* itemObject;
    CCARRAY_FOREACH(m_pChildren, itemObject) {
		CCMenuItem* item = (CCMenuItem*)itemObject;
        // Calculate the position of our menu item. 
		item->setPosition(ccp(col * _padding.x + _pageCount * winSize.width,
                              - row * _padding.y));
        // Increment our positions for the next item(s).
		col++;
		if (col == cols) {
			col = 0;
			row++;
			if(row == rows) {
				_pageCount++;
				col = 0;
				row = 0;
			}
		}
		
	}
    
	if(getChildrenCount() > rows * cols * _pageCount) _pageCount++;
}

void CCSlidingGridMenu::buildVerticalGrid(int cols, int rows) { 
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	int col = 0, row = 0;
	CCObject* itemObject;
	CCARRAY_FOREACH(m_pChildren, itemObject) {
		CCMenuItem* item = (CCMenuItem*)itemObject;
        // Calculate the position of our menu item.
		item->setPosition(ccp(col * _padding.x ,
                              - row * _padding.y - _pageCount * winSize.height));
        // Increment our positions for the next item(s).
		col++;
		if (col == cols) {
			col = 0;
			row++;
			if(row == rows) {
				_pageCount++;
				col = 0;
				row = 0;
			}
		}

	}
    
	if(getChildrenCount() > rows * cols * _pageCount) _pageCount++;
}

CCMenuItem* CCSlidingGridMenu::getItemForTouch(CCTouch* touch) {
    // Get the location of touch.
    CCPoint touchLocation = touch->getLocation();

	// Parse our menu items and see if our touch exists within one.
	CCObject* itemObject;
	CCARRAY_FOREACH(m_pChildren, itemObject) {
		CCMenuItem* item= (CCMenuItem *)itemObject;
		CCPoint local = item->convertToNodeSpace(touchLocation);
		CCRect r = item->rect();
		r.origin = CCPointZero;
        // If the touch was within this item. Return the item.
		if (r.containsPoint(local)) return item;
	}
	// Didn't touch an item. 
	return NULL;
}

void CCSlidingGridMenu::registerWithTouchDispatcher() {
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, INT_MIN+1, true);
}

bool CCSlidingGridMenu::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    // Convert and store the location the touch began at.
    _touchOrigin = touch->getLocation();
    
    // If we weren't in "waiting" state bail out.
	if (_state != kCCMenuStateWaiting) return false;
	
	// Activate the menu item if we are touching one.
	_selectedItem = getItemForTouch(touch);
	if (_selectedItem) {
        _selectedItem->selected();
    }
	
	// Only track touch if we are either in our menu system or dont care if they are outside of the menu grid.
	if (!_isSwipeOnlyOnMenu || (_isSwipeOnlyOnMenu && _selectedItem)) {
		_state = kCCMenuStateTrackingTouch;
		return true;
	}
	
	return false;
}

void CCSlidingGridMenu::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
	if(_selectedItem) {
		_selectedItem->unselected();
		_selectedItem = NULL;
		_state = kCCMenuStateWaiting;
	}
}

void CCSlidingGridMenu::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    _isMoving = true;
	// Calculate the current touch point during the move.
    _touchStop = touch->getLocation();
    // Distance between the origin of the touch and current touch point.
	_moveDistance = (_isVerticalPaging) ? (_touchStop.y - _touchOrigin.y) : (_touchStop.x - _touchOrigin.x);
    // Set our position.
	setPosition(getCurrentPagePosition(_moveDistance));
}

// Touch has ended. Process sliding of menu or press of menu item.
void CCSlidingGridMenu::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    // User has been sliding the menu.
	if(_isMoving) {
		_isMoving = false;
        // Do we have multiple pages?
        if (_pageCount > 1 && (_minMoveDistance < abs(_moveDistance))) {
            // Are we going forward or backward?
            bool isForward = _isVerticalPaging ? _moveDistance > 0 : _moveDistance < 0;
            
            // Do we have a page available?
            if(isForward && (_currentPage < _pageCount - 1)) {
                // Increment currently active page.
                _currentPage++;
            } else if (!isForward && (_currentPage > 0)) {
                // Decrement currently active page.
                _currentPage--;
            }
        }
        // Start sliding towards the current page.
        moveToCurrentPage();
    } else {
        // User wasn't sliding menu and simply tapped the screen. Activate the menu item.
		if(_selectedItem) {
            _selectedItem->unselected();
            _selectedItem->activate();
		}
	}

	// Back to waiting state.
	_state = kCCMenuStateWaiting;
}

void CCSlidingGridMenu::moveToCurrentPage(bool animated) {
    CCPoint currentPagePosition = getCurrentPagePosition();
    if (animated) {
        runAction(CCEaseBounce::create(CCMoveTo::create(0.3f, currentPagePosition)));
    } else {
        setPosition(currentPagePosition);
    }
}

CCPoint CCSlidingGridMenu::getCurrentPagePosition(float offset) {
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();
	return _isVerticalPaging ?
    CCPointMake(_menuOrigin.x, _menuOrigin.y + _currentPage * winSize.height + offset) :
    CCPointMake(_menuOrigin.x - _currentPage * winSize.width + offset, _menuOrigin.y);
}

void CCSlidingGridMenu::moveToPage(int page, bool animated) {
    page = clampf(page, 0, _pageCount - 1);
    if (_currentPage != page) {
        _currentPage = page;
        moveToCurrentPage(animated);
    }
}

NS_CC_END