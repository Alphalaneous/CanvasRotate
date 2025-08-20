#pragma once
#include <Geode/Geode.hpp>

using namespace geode::prelude;

class MultiTouchLayer : public CCLayer {
protected:
    EditorUI* m_editorUI;
    Ref<CCTouch> m_firstTouch;
    Ref<CCTouch> m_secondTouch;
public:
    static MultiTouchLayer* create(EditorUI* editorUI);
    bool init(EditorUI* editorUI);
    bool isSwiping();

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
    void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
    void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override;

};