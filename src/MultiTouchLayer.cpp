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

bool MultiTouchLayer::ccTouchBegan(CCTouch* touch, CCEvent* event) {
    bool isFirst = false;

    if (!m_firstTouch) {
        m_firstTouch = touch;
        isFirst = true;
    }
    else if (!m_secondTouch) {
        m_secondTouch = touch;
    }

    if (isFirst || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchBegan(touch, event);
        return true;
    }

    m_editorUI->m_isDraggingCamera = false;
    m_editorUI->stopActionByTag(123);

    if (auto* editorUI = static_cast<MyEditorUI*>(EditorUI::get()); m_firstTouch != touch) {
        auto fields = editorUI->m_fields.self();
        fields->m_rotateDragging = true;
        fields->m_lastPos = touch->getLocation();
    }

    return true;
}

void MultiTouchLayer::ccTouchMoved(CCTouch* touch, CCEvent* event) {
    if ((!m_secondTouch && m_firstTouch) || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchMoved(touch, event);
        return;
    }
    if (touch == m_firstTouch) {
        return;
    }

    if (auto* editorUI = static_cast<MyEditorUI*>(EditorUI::get())) {
        auto fields = editorUI->m_fields.self();
        if (!fields->m_rotateDragging) return;

        const CCPoint currentPos = touch->getLocation();
        const CCPoint center = (m_firstTouch->getLocation() + currentPos) / 2;

        auto angle = [](const CCPoint& p) {
            return atan2f(p.y, p.x);
        };

        const auto v1 = fields->m_lastPos - center;
        const auto v2 = currentPos - center;

        float deltaAngle = CC_RADIANS_TO_DEGREES(angle(v2) - angle(v1));
        if (deltaAngle > 180.f) deltaAngle -= 360.f;
        if (deltaAngle < -180.f) deltaAngle += 360.f;

        editorUI->updateCanvasRotation(deltaAngle);
        fields->m_lastPos = currentPos;
    }
}

void MultiTouchLayer::ccTouchEnded(CCTouch* touch, CCEvent* event) {
    if (touch == m_firstTouch) m_firstTouch = nullptr;
    else if (touch == m_secondTouch) m_secondTouch = nullptr;

    if (!m_secondTouch || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchEnded(touch, event);
    }
}

void MultiTouchLayer::ccTouchCancelled(CCTouch* touch, CCEvent* event) {
    if (touch == m_firstTouch) m_firstTouch = nullptr;
    else if (touch == m_secondTouch) m_secondTouch = nullptr;

    if (!m_secondTouch || m_editorUI->m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        m_editorUI->ccTouchCancelled(touch, event);
    }
}