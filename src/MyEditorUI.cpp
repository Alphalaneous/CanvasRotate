#include "MyEditorUI.hpp"
#include <alphalaneous.good_grid/include/DrawGridAPI.hpp>
#include <alphalaneous.good_grid/include/DrawLayers.hpp>

bool MyEditorUI::init(LevelEditorLayer* editorLayer) {
    if (!EditorUI::init(editorLayer)) {
        return false;
    }
    auto fields = m_fields.self();
    fields->m_editorLoaded = true;

    if (auto editorButtonsMenu = getChildByID("editor-buttons-menu")) {
        auto spr = CCSprite::createWithSpriteFrameName("gj_navDotBtn_off_001.png");
        auto btn = CCMenuItemSpriteExtra::create(spr, this, menu_selector(MyEditorUI::toggleVanillaDraw));
        btn->setContentSize({40, 40});
        editorButtonsMenu->addChild(btn);
        editorButtonsMenu->updateLayout();
    }

    // evil
    if (auto betterEdit = Loader::get()->getLoadedMod("hjfod.betteredit")) {
        for (auto hook : betterEdit->getHooks()) {
            if (hook->getDisplayName() == "EditorUI::scrollWheel") {
                (void) hook->disable();
                break;
            }
        }
    }
    setTouchMode(ccTouchesMode::kCCTouchesAllAtOnce);
    
    return true;
}

void MyEditorUI::moveObject(GameObject* p0, cocos2d::CCPoint p1) {
    auto fields = m_fields.self();
    if (fields->m_editorLoaded) {
        int rot = static_cast<int>(std::round(fields->m_rotation));
        if (rot < 45 || rot >= 315) {
            p1 = CCPoint{p1.x, p1.y};
        }
        else if (rot < 135) {
            p1 = CCPoint{-p1.y, p1.x};
        }
        else if (rot < 225) {
            p1 = CCPoint{-p1.x, -p1.y};
        }
        else {
            p1 = CCPoint{p1.y, -p1.x};
        }
    }

    EditorUI::moveObject(p0, p1);
}

GameObject* MyEditorUI::createObject(int p0, cocos2d::CCPoint p1) {
    auto ret = EditorUI::createObject(p0, p1);
    auto fields = m_fields.self();
    if (fields->m_editorLoaded) {

    int rot = static_cast<int>(std::round(fields->m_rotation));
        if (rot < 45 || rot >= 315) {
            ret->setRotation(0);
        }
        else if (rot < 135) {
            ret->setRotation(270);
        }
        else if (rot < 225) {
            ret->setRotation(180);
        }
        else {
            ret->setRotation(90);
        }
    }

    return ret;
}

void MyEditorUI::toggleVanillaDraw(CCObject* obj) {
    DrawGridAPI::get().setVanillaDraw(!DrawGridAPI::get().isVanillaDraw());
}

void MyEditorUI::playtestStopped() {
    EditorUI::playtestStopped();
    auto fields = m_fields.self();
    m_editorLayer->m_gameState.m_cameraAngle = fields->m_rotation;
}

void MyEditorUI::updateCanvasRotation(float deltaAngle) {

    if (m_editorLayer->m_playbackMode == PlaybackMode::Playing) return;

    const float snapIncrement = 45.0f;
    #ifdef GEODE_IS_DESKTOP
    const float snapThreshold = 2.0f;
    #else
    const float snapThreshold = 4.0f;
    #endif
    const float unsnapThreshold = 5.0f;
    const float smoothingFactor = 0.2f;
    auto fields = m_fields.self();

    fields->m_unsnappedCameraAngle = std::fmod(fields->m_unsnappedCameraAngle - deltaAngle, 360.0f);
    if (fields->m_unsnappedCameraAngle < 0) fields->m_unsnappedCameraAngle += 360.0f;

    float nearestSnap = std::round(fields->m_unsnappedCameraAngle / snapIncrement) * snapIncrement;
    float diff = std::fabs(fields->m_unsnappedCameraAngle - nearestSnap);

    float targetAngle;

    if (!fields->m_isSnapped && diff < snapThreshold) {
        targetAngle = nearestSnap;
        fields->m_isSnapped = true;
    } 
    else if (fields->m_isSnapped && diff < unsnapThreshold) {
        targetAngle = nearestSnap;
    } 
    else {
        targetAngle = fields->m_unsnappedCameraAngle;
        fields->m_isSnapped = false;
    }

    auto shortestDelta = std::fmod(targetAngle - fields->m_smoothedCameraAngle + 540.0f, 360.0f) - 180.0f;
    fields->m_smoothedCameraAngle = std::fmod(fields->m_smoothedCameraAngle + shortestDelta * smoothingFactor + 360.0f, 360.0f);

    m_editorLayer->m_gameState.m_cameraAngle = fields->m_smoothedCameraAngle;

    fields->m_rotation = m_editorLayer->m_gameState.m_cameraAngle;

    DrawGridAPI::get().setLineSmoothing(static_cast<int>(std::round(fields->m_rotation)) % 90 != 0);

    DrawGridAPI::get().markDirty();
}

void MyEditorUI::clickOnPosition(cocos2d::CCPoint p0) {
    auto oldToolbarHeight = m_toolbarHeight;
    m_toolbarHeight = INT_MIN;
    EditorUI::clickOnPosition(p0);
    m_toolbarHeight = oldToolbarHeight;
};

bool MyEditorUI::isSwiping() {
    return m_swipeEnabled || CCKeyboardDispatcher::get()->getShiftKeyPressed();
}

bool MyEditorUI::ccTouchBegan(cocos2d::CCTouch* touch, cocos2d::CCEvent* p1) {
    auto fields = m_fields.self();
    CCPoint preTransform = touch->getLocation();
    if (isSwiping() && m_selectedMode == 3) {
        return EditorUI::ccTouchBegan(touch, p1);
    }

    fields->m_touchCount++;
    if (!fields->m_firstTouch) {
        fields->m_firstTouch = touch;
    }

    if (fields->m_touchCount <= 1) {
        return EditorUI::ccTouchBegan(touch, p1);
    }
    else {
        m_isDraggingCamera = false;
        stopActionByTag(123);
    }

    if (fields->m_firstTouch != touch) {
        fields->m_rotateDragging = true;
        fields->m_lastPos = touch->getLocation();
        return true;
    }

    translate(touch);
    auto oldToolbarHeight = m_toolbarHeight;
    m_toolbarHeight = INT_MIN;
    if (preTransform.y <= oldToolbarHeight) {
        m_toolbarHeight = oldToolbarHeight;
        return true;
    }
    auto ret = EditorUI::ccTouchBegan(touch, p1);
    m_toolbarHeight = oldToolbarHeight;
    return ret;
}

void MyEditorUI::ccTouchMoved(cocos2d::CCTouch* touch, cocos2d::CCEvent* p1) {
    auto fields = m_fields.self();
    if ((m_swipeEnabled || CCKeyboardDispatcher::get()->getShiftKeyPressed()) && m_selectedMode == 3) {
        return EditorUI::ccTouchMoved(touch, p1);
    }

    if (fields->m_touchCount <= 1) {
        translate(touch);
        return EditorUI::ccTouchMoved(touch, p1);
    }

    if (touch == fields->m_firstTouch || fields->m_touchCount > 2 || m_editorLayer->m_playbackMode == PlaybackMode::Playing || isSwiping()) {
        return;
    }

    if (fields->m_rotateDragging) {
        auto currentPos = touch->getLocation();

        CCPoint center = (fields->m_firstTouch->getLocation() - -currentPos) / 2;

        auto v1 = fields->m_lastPos - center;
        auto v2 = currentPos - center;

        float angle1 = atan2f(v1.y, v1.x);
        float angle2 = atan2f(v2.y, v2.x);
        float deltaAngle = CC_RADIANS_TO_DEGREES(angle2 - angle1);

        if (deltaAngle > 180.f) deltaAngle -= 360.f;
        if (deltaAngle < -180.f) deltaAngle += 360.f;

        updateCanvasRotation(deltaAngle);

        fields->m_lastPos = currentPos;
    }
}

void MyEditorUI::ccTouchEnded(cocos2d::CCTouch* touch, cocos2d::CCEvent* p1) {
    auto fields = m_fields.self();

    fields->m_touchCount--;

    if (touch == fields->m_firstTouch) {
        fields->m_firstTouch = nullptr;
    }

    translate(touch);
    EditorUI::ccTouchEnded(touch, p1);
}

void MyEditorUI::ccTouchCancelled(cocos2d::CCTouch* touch, cocos2d::CCEvent* p1) {
    auto fields = m_fields.self();

    fields->m_touchCount--;

    if (touch == fields->m_firstTouch) {
        fields->m_firstTouch = nullptr;
    }

    translate(touch);
    EditorUI::ccTouchCancelled(touch, p1);
}

void MyEditorUI::scrollWheel(float y, float x) {
    if (CCKeyboardDispatcher::get()->getShiftKeyPressed()) {
        x = y;
        y = 0;
    }
    float rot = m_fields->m_rotation;

    if (CCKeyboardDispatcher::get()->getControlKeyPressed()) {
        auto layer = m_editorLayer->m_objectLayer;
        float currentScale = layer->getScale();
        CCSize winSize = CCDirector::get()->getWinSize();
        CCPoint mousePos = rotatePointAroundPivot(getMousePos(), winSize/2, rot);
        CCPoint offset = mousePos - layer->getPosition();

        float zoomFactor = 1.05f;
        float zoomSpeed = 0.2f;

        float newScale = currentScale * powf(zoomFactor, -y * zoomSpeed);
        newScale = std::min(std::max(newScale, 0.1f), 4.0f);

        float scaleRatio = newScale / currentScale;
        CCPoint newPos = mousePos - offset * scaleRatio;

        layer->setScale(newScale);
        layer->setPosition(newPos);

        updateZoom(newScale);
        return;
    }

    CCPoint newPos = rotatePointAroundPivot({-x, y}, {0, 0}, rot);

    EditorUI::scrollWheel(newPos.y, newPos.x);
}

void MyEditorUI::translate(cocos2d::CCTouch* touch) {
    CCSize winSize = CCDirector::get()->getWinSize();
    auto fields = m_fields.self();

    CCPoint newPoint = rotatePointAroundPivot(touch->getLocation(), winSize/2, fields->m_rotation);
    touch->setTouchInfo(touch->getID(), newPoint.x, winSize.height - newPoint.y);
}

CCPoint MyEditorUI::rotatePointAroundPivot(CCPoint point, CCPoint pivot, float angleDegrees) {
    float angleRadians = CC_DEGREES_TO_RADIANS(angleDegrees);

    float sinA = sinf(angleRadians);
    float cosA = cosf(angleRadians);

    point.x -= pivot.x;
    point.y -= pivot.y;

    float xNew = point.x * cosA - point.y * sinA;
    float yNew = point.x * sinA + point.y * cosA;

    return CCPoint(xNew + pivot.x, yNew + pivot.y);
}