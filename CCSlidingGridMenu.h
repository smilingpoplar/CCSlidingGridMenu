#ifndef __SLIDING_MENU_GRID_H
#define __SLIDING_MENU_GRID_H
/*
 * a sliding grid menu like Angry Birds
 *
 * 2011-01-09 Brandon - http://brandonreynolds.com/blog/2011/01/09/cocos2d-sliding-menu-grid/
 * 2012-12-16 YangLe  - fix bugs & add features: pages indicator, touch area
 */

#include "cocos2d.h"

NS_CC_BEGIN

class CCSlidingGridMenu;
class CCSlidingGridMenuDelegate {
public:
    virtual void slidingGridMenuDidMoveToPage(CCSlidingGridMenu *menu, int page) = 0;
};

typedef struct {
    float top, left, bottom, right;
} CCEdgeInsets;

class CCSlidingGridMenu : public CCLayer {
public:
    // previewLength
    //  >=0: next/previous page's preview window length
    //  < 0: pages will be next to each other
    static CCSlidingGridMenu* create(CCArray *items, int cols, int rows, const CCSize &itemSize, bool horizontal,
                                     const CCPoint &position, float previewLength = -1);
	void moveToPage(int page, bool animated = true);
    
    // delegate
    CC_SYNTHESIZE(CCSlidingGridMenuDelegate *, _delegate, Delegate);
    
    // pages indicator
    CC_SYNTHESIZE(bool, _showIndicator, ShowIndicator);
    CC_SYNTHESIZE_READONLY(float, _indicatorSize, IndicatorSize);
    void setIndicatorSize(float indicatorSize);
    CC_SYNTHESIZE_PASS_BY_REF(CCPoint, _indicatorPosition, IndicatorPosition);
    CC_SYNTHESIZE_READONLY(CCSprite *, _indicatorSprite, IndicatorSprite);
    void setIndicatorSprite(CCSprite *sprite);
    CC_SYNTHESIZE_PASS_BY_REF(ccColor4B, _indicatorColorNormal, IndicatorColorNormal);
    CC_SYNTHESIZE_PASS_BY_REF(ccColor4B, _indicatorColorSelected, IndicatorColorSelected);
    
    // touch area
    CC_SYNTHESIZE(CCRect, _touchArea, TouchArea);
    void setTouchAreaWithCapInsets(const CCEdgeInsets &capInsets);
protected:
    bool init(CCArray *items, int cols, int rows, const CCSize &itemSize, bool horizontal,
              
              const CCPoint &position, float previewLength);
    virtual void addChild(CCNode *child);
	void buildGrid(int cols, int rows, bool horizontal);
	void moveToCurrentPage(bool animated = true);
    void didMoveToCurrentPage();
    
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
