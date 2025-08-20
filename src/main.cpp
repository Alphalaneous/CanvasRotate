#include <Geode/Geode.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <alphalaneous.good_grid/include/DrawGridAPI.hpp>
#include <alphalaneous.good_grid/include/DrawLayers.hpp>
#include "MultiTouchLayer.hpp"
#include "MyEditorUI.hpp"

using namespace geode::prelude;

	/*
		todo:
		2 finger rotate for mobile
		hold space to align object to view rotation
		rotate arrow keys depending on canvas rotation
	*/

static void forEachObject(GJBaseGameLayer const* game, std::function<void(GameObject*)> const& callback) {
	int count = game->m_sections.empty() ? -1 : game->m_sections.size();
	for (int i = game->m_leftSectionIndex; i <= game->m_rightSectionIndex && i < count; ++i) {
		auto leftSection = game->m_sections[i];
		if (!leftSection) continue;

		auto leftSectionSize = leftSection->size();
		for (int j = game->m_bottomSectionIndex; j <= game->m_topSectionIndex && j < leftSectionSize; ++j) {
			auto section = leftSection->at(j);
			if (!section) continue;

			auto sectionSize = game->m_sectionSizes[i]->at(j);
			for (int k = 0; k < sectionSize; ++k) {
				auto obj = section->at(k);
				if (!obj) continue;

				callback(obj);
			}
		}
	}
}

class $modify(MyLevelEditorLayer, LevelEditorLayer) {

    bool init(GJGameLevel* p0, bool p1) {
		if (!LevelEditorLayer::init(p0, p1)) return false;

		auto touchLayer = MultiTouchLayer::create(m_editorUI);
		touchLayer->setZOrder(500);
		addChild(touchLayer);

		return true;
	}

    cocos2d::CCArray* objectsInRect(cocos2d::CCRect rect, bool useGroupValidity) {
		auto result = cocos2d::CCArray::create();

		cocos2d::CCPoint center = rect.origin + cocos2d::CCPoint(rect.size.width * 0.5f, rect.size.height * 0.5f);
    	OBB2D* selectionOBB = OBB2D::create(center, rect.size.width, rect.size.height, 0);
		
		CCSize winSize = CCDirector::get()->getWinSize();

		CCPoint centerInObjectLayer = m_objectLayer->convertToNodeSpace(winSize/2);

		forEachObject(this, [this, &rect, result, selectionOBB, &winSize, &centerInObjectLayer] (GameObject* object) {

			if (object->getOrientedBox() && selectionOBB->overlaps(rotatedOBB2D(object, centerInObjectLayer, m_gameState.m_cameraAngle))) {
				result->addObject(object);
			}
		});

		return result;
	}

	OBB2D* rotatedOBB2D(GameObject* object, cocos2d::CCPoint pivot, float degrees) {
		auto original = object->getOrientedBox();
		if (!original) return nullptr;

		auto center = original->m_center;

		auto& c0 = original->m_corners[0];
		auto& c1 = original->m_corners[1];
		auto& c2 = original->m_corners[2]; 

		float width = (c1 - c0).getLength();
		float height = (c2 - c1).getLength();
		float angle = CC_RADIANS_TO_DEGREES(std::atan2(c1.y - c0.y, c1.x - c0.x));

		float radians = -CC_DEGREES_TO_RADIANS(degrees);
		float dx = center.x - pivot.x;
		float dy = center.y - pivot.y;

		cocos2d::CCPoint rotatedCenter = {
			pivot.x + dx * std::cos(radians) - dy * std::sin(radians),
			pivot.y + dx * std::sin(radians) + dy * std::cos(radians)
		};

		return OBB2D::create(rotatedCenter, width, height, angle + degrees);
	}
};

#ifdef GEODE_IS_WINDOWS

class $modify(MyCCEGLView, CCEGLView) {

	void onGLFWMouseCallBack(GLFWwindow* window, int button, int action, int mods) {
		CCEGLView::onGLFWMouseCallBack(window, button, action, mods);
		if (MyEditorUI* editorUI = static_cast<MyEditorUI*>(EditorUI::get())) {
			auto fields = editorUI->m_fields.self();
			if (button == GLFW_MOUSE_BUTTON_RIGHT) {
				fields->m_rotateDragging = action;
				if (action == 1) {
					fields->m_lastPos = getMousePos();
				}
			}
		}
	}

	void onGLFWMouseMoveCallBack(GLFWwindow* window, double x, double y) {
		CCEGLView::onGLFWMouseMoveCallBack(window, x, y);
		
		if (MyEditorUI* editorUI = static_cast<MyEditorUI*>(EditorUI::get())) {
			auto fields = editorUI->m_fields.self();
			if (fields->m_rotateDragging) {

				auto currentPos = getMousePos();
				auto screenCenter = CCDirector::sharedDirector()->getWinSize() / 2;

				auto v1 = fields->m_lastPos - screenCenter;
    			auto v2 = currentPos - screenCenter;

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
};

#endif