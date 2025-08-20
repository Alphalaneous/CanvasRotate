#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

class $modify(MyEditorUI, EditorUI) {

	static void onModify(auto& self) {
        (void) self.setHookPriorityPre("EditorUI::ccTouchBegan", Priority::EarlyPre);
        (void) self.setHookPriorityPre("EditorUI::ccTouchMoved", Priority::EarlyPre);
        (void) self.setHookPriorityPre("EditorUI::ccTouchEnded", Priority::EarlyPre);
        (void) self.setHookPriorityPre("EditorUI::ccTouchCancelled", Priority::EarlyPre);
        (void) self.setHookPriorityPre("EditorUI::scrollWheel", Priority::EarlyPre);
    }

	struct Fields {
		bool m_rotateDragging = false;
		float m_rotation;
		double m_lastY;
		CCPoint m_lastPos;
		bool m_isSnapped;
		bool m_editorLoaded = false;
		float m_unsnappedCameraAngle = 0.0f;
		float m_smoothedCameraAngle = 0.0f;
		int m_touchCount;
		Ref<CCTouch> m_firstTouch;
	};

    bool init(LevelEditorLayer* editorLayer);
    void moveObject(GameObject* p0, cocos2d::CCPoint p1);
    GameObject* createObject(int p0, cocos2d::CCPoint p1);
	void toggleVanillaDraw(CCObject* obj);
    void playtestStopped();
	void updateCanvasRotation(float deltaAngle);
    void clickOnPosition(cocos2d::CCPoint p0);
	bool ccTouchBegan(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1);
    void ccTouchMoved(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1);
    void ccTouchEnded(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1);
    void ccTouchCancelled(cocos2d::CCTouch* p0, cocos2d::CCEvent* p1);
	void scrollWheel(float y, float x);
	void translate(cocos2d::CCTouch* touch);
	bool isSwiping();
	CCPoint rotatePointAroundPivot(CCPoint point, CCPoint pivot, float angleDegrees);
};
