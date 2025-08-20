#include "MultiTouchLayer.hpp"
#include "MyEditorUI.hpp"

MultiTouchLayer* MultiTouchLayer::create(EditorUI* editorUI) {
    auto ret = new MultiTouchLayer();
    if (ret->init(editorUI)) {
        ret->autorelease();
        return ret;
    }
    delete ret;
    return nullptr;
}

bool MultiTouchLayer::init(EditorUI* editorUI) {
    if (!CCLayer::init()) return false;
    m_editorUI = editorUI;

    auto winSize = CCDirector::get()->getWinSize();

    setAnchorPoint({0.5f, 0.5f});
    ignoreAnchorPointForPosition(false);
    setContentSize(winSize);
    setPosition(winSize/2);

    setTouchMode(ccTouchesMode::kCCTouchesAllAtOnce);
    setTouchEnabled(true);

    cocos2d::CCTouchDispatcher::get()->addTargetedDelegate(this, cocos2d::kCCMenuHandlerPriority, true);

    return true;
}

bool MultiTouchLayer::isSwiping() {
    return (m_editorUI->m_swipeEnabled || CCKeyboardDispatcher::get()->getShiftKeyPressed()) && m_editorUI->m_selectedMode == 3;
}

bool MultiTouchLayer::ccTouchBegan(CCTouch *pTouch, CCEvent *pEvent) {
    m_touchCount++;
    if (!m_firstTouch) {
        m_firstTouch = pTouch;
    }

    if (m_touchCount <= 1 || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchBegan(pTouch, pEvent);
        return true;
    }
    else {
        m_editorUI->m_isDraggingCamera = false;
        m_editorUI->stopActionByTag(123);
    }

    if (MyEditorUI* editorUI = static_cast<MyEditorUI*>(EditorUI::get())) {
        if (m_firstTouch != pTouch) {
            auto fields = editorUI->m_fields.self();
            fields->m_rotateDragging = true;
            fields->m_lastPos = pTouch->getLocation();
        }
    }

    return true;
}

void MultiTouchLayer::ccTouchMoved(CCTouch *pTouch, CCEvent *pEvent) {
    if (m_touchCount <= 1 || m_touchCount > 2 || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchMoved(pTouch, pEvent);
        return;
    }

    if (pTouch == m_firstTouch) {
        return;
    }

    if (MyEditorUI* editorUI = static_cast<MyEditorUI*>(EditorUI::get())) {
        auto fields = editorUI->m_fields.self();
        if (fields->m_rotateDragging) {

            auto currentPos = pTouch->getLocation();

            CCPoint center = (m_firstTouch->getLocation() - -currentPos) / 2;

            auto v1 = fields->m_lastPos - center;
            auto v2 = currentPos - center;

            float angle1 = atan2f(v1.y, v1.x);
            float angle2 = atan2f(v2.y, v2.x);
            float deltaAngle = CC_RADIANS_TO_DEGREES(angle2 - angle1);

            if (deltaAngle > 180.f) deltaAngle -= 360.f;
            if (deltaAngle < -180.f) deltaAngle += 360.f;

            editorUI->updateCanvasRotation(deltaAngle);

            fields->m_lastPos = currentPos;
        }
    }
}

void MultiTouchLayer::ccTouchEnded(CCTouch *pTouch, CCEvent *pEvent) {
    m_touchCount--;

    if (pTouch == m_firstTouch) {
        m_firstTouch = nullptr;
    }

    if (m_touchCount < 1 || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchEnded(pTouch, pEvent);
        return;
    }
}

void MultiTouchLayer::ccTouchCancelled(CCTouch *pTouch, CCEvent *pEvent) {
    m_touchCount--;

    if (pTouch == m_firstTouch) {
        m_firstTouch = nullptr;
    }

    if (m_touchCount < 1 || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchCancelled(pTouch, pEvent);
        return;
    }

}