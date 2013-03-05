#include "CCSlidingGridMenu.h"
#include <vector>

NS_CC_BEGIN

CCSlidingGridMenu* CCSlidingGridMenu::create(CCArray *items, int cols, int rows, const CCSize &itemSize,
                                             bool horizontal, const CCPoint &position, float previewLength) {
	CCSlidingGridMenu *slidingMenu = new CCSlidingGridMenu();
	if (slidingMenu && slidingMenu->init(items, cols, rows, itemSize, horizontal, position, previewLength)) {
		slidingMenu->autorelease();
		return slidingMenu;
	}
	CC_SAFE_DELETE(slidingMenu);
	return NULL;
}

bool CCSlidingGridMenu::init(CCArray *items, int cols, int rows, const CCSize &itemSize, bool horizontal,
                             const CCPoint &position, float previewLength) {
	if(!CCLayer::init()) return false;
    
	setTouchEnabled(true);
    
    CCObject *itemObject;
    CCARRAY_FOREACH(items, itemObject) {
        CCMenuItem *item = (CCMenuItem*)itemObject;
        addChild(item);
    }
    
	_state = kCCMenuStateWaiting;
	_selectedItem = NULL;
	_currentPage = 0;
	_moving = false;
    
    _itemSize = itemSize;
	_menuOrigin = position;
	_horizontal = horizontal;
    
    CCSize winSize(CCDirector::sharedDirector()->getWinSize());
    float winLength = horizontal ? winSize.width : winSize.height;
    CCSize menuSize(_itemSize.width * cols, _itemSize.height * rows);
    if (previewLength < 0) {
        float menuLength = horizontal ? menuSize.width : menuSize.height;
        previewLength = winLength - menuLength;
    }
    _pageOffset = winLength - previewLength;
    
	setPosition(position);
	buildGrid(cols, rows, horizontal);
    
    _delegate = NULL;
    
    _showIndicator = false;
    _indicatorSize = 6;
    _indicatorPosition = ccp(position.x, position.y - menuSize.height * 0.5 - _indicatorSize * 3);
    _indicatorSprite = NULL;
    _indicatorColorNormal = ccc4(150, 150, 150, 255);
    _indicatorColorSelected = ccc4(255, 255, 255, 255);
    
	return true;
}

void CCSlidingGridMenu::addChild(CCNode *child) {
    CCAssert(dynamic_cast<CCMenuItem*>(child) != NULL, "Menu only supports MenuItem objects as children");
    CCLayer::addChild(child);
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
    CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, 0, true);
}

bool CCSlidingGridMenu::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    // Convert and store the location the touch began at.
    _touchOrigin = touch->getLocation();
    
    // If we weren't in "waiting" state bail out.
	if (_state != kCCMenuStateWaiting) return false;
    
    // If we are not in the specific touch area
    if (!_touchArea.equals(CCRectZero) &&!_touchArea.containsPoint(_touchOrigin)) return false;
	
	// Activate the menu item if we are touching one.
	_selectedItem = getItemForTouch(touch);
	if (_selectedItem) {
        _selectedItem->selected();
    }
	
    _state = kCCMenuStateTrackingTouch;
    return true;
}

void CCSlidingGridMenu::ccTouchMoved(CCTouch* touch, CCEvent* event) {
	// Calculate the current touch point during the move.
    _touchStop = touch->getLocation();
    // Distance between the origin of the touch and current touch point.
	_moveDistance = _horizontal ? _touchStop.x - _touchOrigin.x : _touchStop.y - _touchOrigin.y;
    // Set our position.
	setPosition(getCurrentPagePosition(_moveDistance));
    _moving = true;
    if(_selectedItem) {
        _selectedItem->unselected();
        _selectedItem = NULL;
    }
}

// Touch has ended. Process sliding of menu or press of menu item.
void CCSlidingGridMenu::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    // User wasn't sliding menu and simply tapped the screen. Activate the menu item.
    if (!_moving) {
		if(_selectedItem) {
            _selectedItem->unselected();
            _selectedItem->activate();
            _selectedItem = NULL;
		}
        // Back to waiting state.
        _state = kCCMenuStateWaiting;
        return;
    }
    
    // User has been sliding the menu.
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
        CCFiniteTimeAction *action = CCEaseBounce::create(CCMoveTo::create(0.3f, currentPagePosition));
        action = CCSequence::create(action, CCCallFunc::create(this, callfunc_selector(CCSlidingGridMenu::didMoveToCurrentPage)), NULL);
        runAction(action);
    } else {
        setPosition(currentPagePosition);
        didMoveToCurrentPage();
    }
}

void CCSlidingGridMenu::didMoveToCurrentPage() {
    if (dynamic_cast<CCSlidingGridMenuDelegate *>(_delegate)) {
        _delegate->slidingGridMenuDidMoveToPage(this, _currentPage);
    }
}

CCPoint CCSlidingGridMenu::getCurrentPagePosition(float offset) {
	return _horizontal ?
    ccp(_menuOrigin.x - _currentPage * _pageOffset + offset, _menuOrigin.y) :
    ccp(_menuOrigin.x, _menuOrigin.y + _currentPage * _pageOffset + offset);
}

void CCSlidingGridMenu::moveToPage(int page, bool animated) {
    page = MIN(MAX(0, page), _pageCount - 1);
    if (page != _currentPage) {
        _currentPage = page;
        moveToCurrentPage(animated);
    }
}

static void setColor4B(CCSprite *sprite, const ccColor4B &color) {
    sprite->setColor(ccc3(color.r, color.g, color.b));
    sprite->setOpacity(color.a);
}

static void drawSprite(CCSprite *sprite, const CCPoint &point) {
    sprite->setPosition(point);
    sprite->visit();
}

void CCSlidingGridMenu::visit() {
    CCLayer::visit();
	
	if (_showIndicator) {
		// Prepare Points Array
		float distanceBetweenPoints = _indicatorSize * 2;
        std::vector<CCPoint> points(_pageCount);
		for (int i = 0; i < _pageCount; ++i) {
			points[i] = ccp(_indicatorPosition.x + distanceBetweenPoints * (i - 0.5f*(_pageCount-1)), _indicatorPosition.y);
		}
		
        if (_indicatorSprite) {
            _indicatorSprite->setScale(_indicatorSize / _indicatorSprite->getContentSize().width);
            setColor4B(_indicatorSprite, _indicatorColorNormal);
            for (int i = 0; i < _pageCount; i++) {
                drawSprite(_indicatorSprite, points[i]);
            }
            setColor4B(_indicatorSprite, _indicatorColorSelected);
            drawSprite(_indicatorSprite, points[_currentPage]);
        } else {
            // Set GL Values
            ccGLEnable(CC_GL_BLEND);
            ccPointSize(_indicatorSize);
            
            // Draw Gray Points
            ccDrawColor4B(_indicatorColorNormal.r,
                          _indicatorColorNormal.g,
                          _indicatorColorNormal.b,
                          _indicatorColorNormal.a);
            ccDrawPoints(points.data(), _pageCount);
            
            // Draw White Point for Selected Page
            ccDrawColor4B(_indicatorColorSelected.r,
                          _indicatorColorSelected.g,
                          _indicatorColorSelected.b,
                          _indicatorColorSelected.a);
            ccDrawPoint(points[_currentPage]);
            
            // Restore GL Values
            ccPointSize(1.0f);
        }
    }
}

void CCSlidingGridMenu::setIndicatorSprite(CCSprite *sprite) {
    if (_indicatorSprite != sprite) {
        CC_SAFE_RETAIN(sprite);
        CC_SAFE_RELEASE(_indicatorSprite);
        _indicatorSprite = sprite;
        
        if (sprite) sprite->setScale(_indicatorSize / sprite->getContentSize().width);
    }
}

void CCSlidingGridMenu::setTouchAreaWithCapInsets(const CCEdgeInsets &capInsets) {
    const CCSize &winSize = CCDirector::sharedDirector()->getWinSize();
    _touchArea =  CCRectMake(capInsets.left,
                             capInsets.bottom,
                             winSize.width - capInsets.left - capInsets.right,
                             winSize.height - capInsets.bottom - capInsets.top);
}

NS_CC_END