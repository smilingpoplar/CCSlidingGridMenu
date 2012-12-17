#ifndef __SLIDING_MENU_GRID_H
#define __SLIDING_MENU_GRID_H
/*
 * port Brandon‘s SlidingMenuGrid to cocos2d-x v2.x
 * (http://brandonreynolds.com/blog/2011/01/09/cocos2d-sliding-menu-grid/ )
 *
 * 2012-12-16 YangLe - add pages indicator
 */

#include "cocos2d.h"

NS_CC_BEGIN

class CCSlidingGridMenu : public CCLayer {
public:
    // previewOffset
    //  >=0: preview offset of the next/previous page
    //  < 0: pages will be next to each other
    static CCSlidingGridMenu* create(CCArray *items, int cols, int rows, const CCSize &itemSize, const CCPoint &position, bool horizontal, float previewOffset = -1);
	void moveToPage(int page, bool animated = true);
    void setIndicatorSprite(CCSprite *sprite);
    
    // pages indicator
    CC_SYNTHESIZE(bool, _showIndicator, ShowIndicator);
    CC_SYNTHESIZE(float, _indicatorSize, IndicatorSize);
    CC_SYNTHESIZE_PASS_BY_REF(CCPoint, _indicatorPosition, IndicatorPosition);
    CC_SYNTHESIZE_READONLY(CCSprite *, _indicatorSprite, IndicatorSprite);
    CC_SYNTHESIZE_PASS_BY_REF(ccColor4B, _indicatorColorNormal, IndicatorColorNormal);
    CC_SYNTHESIZE_PASS_BY_REF(ccColor4B, _indicatorColorSelected, IndicatorColorSelected);
    
protected:
    bool init(CCArray *items, int cols, int rows, const CCSize &itemSize, const CCPoint &position, bool horizontal, float previewOffset);
    virtual void addChild(CCNode *child, int zOrder, int tag);
	void buildGrid(int cols, int rows, bool horizontal);
	void moveToCurrentPage(bool animated = true);

    virtual void registerWithTouchDispatcher();
	virtual bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent);
	virtual void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent);
	virtual void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent);
	virtual void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent);
	CCPoint getCurrentPagePosition(float offset = 0);
	CCMenuItem* getItemForTouch(CCTouch* touch);
    
    virtual void visit();

protected:
	tCCMenuState _state; // State of our menu grid. (Eg. waiting, tracking touch, cancelled, etc)
	CCMenuItem *_selectedItem; // Menu item that was selected/active.

	CCSize _itemSize; // size of the menu item.
	CCPoint _menuOrigin; // Origin position of the entire menu grid.
	CCPoint _touchOrigin; // Where the touch action began.
	CCPoint _touchStop; // Where the touch action stopped.

	int _pageCount; // Number of pages in this grid.
	int _currentPage; // Current page of menu items being viewed.
	bool _horizontal; // Scroll pages horizontally?
    float _pageOffset; // Offset between pages. 

	bool _moving; // Is the grid currently moving?
	float _moveDistance; // Distance between origin of the touch and current frame.
};

NS_CC_END

#endif
