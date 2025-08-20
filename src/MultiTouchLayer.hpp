#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class MultiTouchLayer : public CCLayer {
protected:
    EditorUI* m_editorUI;
    int m_touchCount;
    Ref<CCTouch> m_firstTouch;
public:
    static MultiTouchLayer* create(EditorUI* editorUI);
    bool init(EditorUI* editorUI);
    bool isSwiping();

    bool ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) override;
    void ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) override;
    void ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) override;
    void ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) override;

};