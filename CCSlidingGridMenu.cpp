#include "CCSlidingGridMenu.h"

NS_CC_BEGIN

CCSlidingGridMenu* CCSlidingGridMenu::create(CCArray *items, int cols, int rows, const CCSize &itemSize, const CCPoint &position, bool horizontal, float previewOffset) {
	CCSlidingGridMenu *slidingMenu = new CCSlidingGridMenu();
	if (slidingMenu && slidingMenu->init(items, cols, rows, itemSize, position, horizontal, previewOffset)) {
		slidingMenu->autorelease();
		return slidingMenu;
	}
	CC_SAFE_DELETE(slidingMenu);
	return NULL;
}

bool CCSlidingGridMenu::init(CCArray *items, int cols, int rows, const CCSize &itemSize, const CCPoint &position, bool horizontal, float previewOffset) {
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
	_currentPage = 0;
	_moving = false;
    
    _itemSize = itemSize;
	_menuOrigin = position;
	_horizontal = horizontal;
    CCSize winSize = CCDirector::sharedDirector()->getWinSize();
    _pageOffset = horizontal ? winSize.width : winSize.height;
    CCSize menuSize = CCSizeMake(_itemSize.width * cols, _itemSize.height * rows);
    if (previewOffset < 0) {
        previewOffset = _pageOffset - (horizontal ? menuSize.width * 0.5 : menuSize.height * 0.5);
    }
    _pageOffset -=  previewOffset;
    
	setPosition(position);
	buildGrid(cols, rows, horizontal);
    
    _showPagesIndicator = true;
    _pagesIndicatorSize = 6;
    _pagesIndicatorPosition = ccp(position.x, position.y - menuSize.height * 0.5 - _pagesIndicatorSize * 2);
    _pagesIndicatorColorNormal = ccc4(150, 150, 150, 255);
    _pagesIndicatorColorSelected = ccc4(255, 255, 255, 255);
    
	return true;
}

void CCSlidingGridMenu::addChild(CCNode *child, int zOrder, int tag) {
    CCAssert(dynamic_cast<CCMenuItem*>(child) != NULL, "Menu only supports MenuItem objects as children");
    CCLayer::addChild(child, zOrder, tag);
}

void CCSlidingGridMenu::buildGrid(int cols, int rows, bool horizontal) {
	_pageCount = 0;
    
    CCPoint offset = ccp(-_itemSize.width * (cols-1) * 0.5, _itemSize.height * (rows-1) * 0.5);
	int col = 0, row = 0;
	CCObject* itemObject;
	CCARRAY_FOREACH(m_pChildren, itemObject) {
		CCMenuItem* item = (CCMenuItem*)itemObject;
        // Calculate the position of our menu item.
        CCPoint position = horizontal ?
        ccp(offset.x + col * _itemSize.width + _pageCount * _pageOffset, offset.y - row * _itemSize.height) :
        ccp(offset.x + col * _itemSize.width, offset.y - row * _itemSize.height - _pageCount * _pageOffset) ;
        item->setPosition(position);

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
	
    _state = kCCMenuStateTrackingTouch;
    return true;
}

void CCSlidingGridMenu::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    _moving = true;
	// Calculate the current touch point during the move.
    _touchStop = touch->getLocation();
    // Distance between the origin of the touch and current touch point.
	_moveDistance = _horizontal ? _touchStop.x - _touchOrigin.x : _touchStop.y - _touchOrigin.y;
    // Set our position.
	setPosition(getCurrentPagePosition(_moveDistance));
}

// Touch has ended. Process sliding of menu or press of menu item.
void CCSlidingGridMenu::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    // User has been sliding the menu.
	if(_moving) {
		_moving = false;
        // Do we have multiple pages?
        if (_pageCount > 1 && abs(_moveDistance) > _pageOffset * 0.33) {
            // Are we going forward or backward?
            bool forward = _horizontal ? _moveDistance < 0 : _moveDistance > 0;
            
            // Do we have a page available?
            if(forward && (_currentPage < _pageCount - 1)) {
                // Increment currently active page.
                _currentPage++;
            } else if (!forward && (_currentPage > 0)) {
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

void CCSlidingGridMenu::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
	if(_selectedItem) {
		_selectedItem->unselected();
		_selectedItem = NULL;
		_state = kCCMenuStateWaiting;
	}
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
	return _horizontal ?
    ccp(_menuOrigin.x - _currentPage * _pageOffset + offset, _menuOrigin.y) :
    ccp(_menuOrigin.x, _menuOrigin.y + _currentPage * _pageOffset + offset);
}

void CCSlidingGridMenu::moveToPage(int page, bool animated) {
    CCAssert(0 <= page && page < _pageCount, "page out of bounds");
    if (page != _currentPage) {
        _currentPage = page;
        moveToCurrentPage(animated);
    }
}

void CCSlidingGridMenu::visit() {
    CCLayer::visit();
	
	if (_showPagesIndicator) {		
		// Prepare Points Array
		float pY = _pagesIndicatorPosition.y;
		float d = _pagesIndicatorSize * 2; // Distance between points.
        CCPoint *points = new CCPoint[_pageCount];
		for (int i = 0; i < _pageCount; ++i) {
			float pX = _pagesIndicatorPosition.x + d * ( (float)i - 0.5f*(_pageCount-1) );
			points[i] = ccp(pX, pY);
		}
		
		// Set GL Values
        ccGLEnable(CC_GL_BLEND);
        ccPointSize(_pagesIndicatorSize);
 		
 		// Draw Gray Points
        ccDrawColor4B(_pagesIndicatorColorNormal.r,
                      _pagesIndicatorColorNormal.g,
                      _pagesIndicatorColorNormal.b,
                      _pagesIndicatorColorNormal.a);
        ccDrawPoints(points, _pageCount);
        
        // Draw White Point for Selected Page
        ccDrawColor4B(_pagesIndicatorColorSelected.r,
                      _pagesIndicatorColorSelected.g,
                      _pagesIndicatorColorSelected.b,
                      _pagesIndicatorColorSelected.a);
        ccDrawPoint(points[_currentPage]);
        
        // Restore GL Values
        ccPointSize(1.0f);
        
        delete[] points;
    }
}

NS_CC_END