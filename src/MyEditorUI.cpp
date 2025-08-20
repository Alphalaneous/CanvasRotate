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

bool MyEditorUI::ccTouchBegan(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1) {
    CCPoint preTransform = p0->getLocation();
    if ((m_swipeEnabled || CCKeyboardDispatcher::get()->getShiftKeyPressed()) && m_selectedMode == 3) {
        return EditorUI::ccTouchBegan(p0, p1);
    }
    translate(p0);
    auto oldToolbarHeight = m_toolbarHeight;
    m_toolbarHeight = INT_MIN;
    if (preTransform.y <= oldToolbarHeight) {
        m_toolbarHeight = oldToolbarHeight;
        return true;
    }
    auto ret = EditorUI::ccTouchBegan(p0, p1);
    m_toolbarHeight = oldToolbarHeight;
    return ret;
}

void MyEditorUI::ccTouchMoved(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1) {
    if ((m_swipeEnabled || CCKeyboardDispatcher::get()->getShiftKeyPressed()) && m_selectedMode == 3) {
        return EditorUI::ccTouchMoved(p0, p1);
    }
    translate(p0);
    EditorUI::ccTouchMoved(p0, p1);
}

void MyEditorUI::ccTouchEnded(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1) {
    translate(p0);
    EditorUI::ccTouchEnded(p0, p1);
}

void MyEditorUI::ccTouchCancelled(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1) {
    translate(p0);
    EditorUI::ccTouchCancelled(p0, p1);
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