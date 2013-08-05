// Copyright (c) 2013 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: http://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Assets.h"
#include "SSVOpenHexagon/Utils/Utils.h"
#include "SSVOpenHexagon/Core/HexagonGame.h"

using namespace std;
using namespace sf;
using namespace ssvs;
using namespace sses;
using namespace hg::Utils;
using namespace ssvu;

namespace hg
{
	void HexagonGame::draw()
	{
		styleData.computeColors();

		window.clear(Color::Black);
		if(!getNoBackground()) { backgroundCamera.apply(); styleData.drawBackground(window.getRenderWindow(), {0, 0}, getSides()); }
		if(get3D())
		{
			status.drawing3D = true;

			float effect{styleData._3dSkew * get3DMultiplier() * status.pulse3D};
			Vec2f skew{1.f, 1.f + effect};
			backgroundCamera.setSkew(skew);

			for(unsigned int i{0}; i < depthCameras.size(); ++i)
			{
				Camera& depthCamera(depthCameras[i]);
				depthCamera.setView(backgroundCamera.getView());
				depthCamera.setSkew(skew);
				depthCamera.setOffset({0, styleData._3dSpacing * (i * styleData._3dPerspectiveMult) * (effect * 3.6f)});
			}

			for(unsigned int i{0}; i < depthCameras.size(); ++i)
			{
				status.overrideColor = getColorDarkened(styleData.get3DOverrideColor(), styleData._3dDarkenMult);
				status.overrideColor.a /= styleData._3dAlphaMult;
				status.overrideColor.a -= i * styleData._3dAlphaFalloff;

				depthCameras[i].apply();
				manager.draw();
			}
			status.drawing3D = false;
		}
		backgroundCamera.apply(); manager.draw();
		overlayCamera.apply(); drawText();

		if(getFlash()) render(flashPolygon);
		if(mustTakeScreenshot) { window.getRenderWindow().capture().saveToFile("screenshot.png"); mustTakeScreenshot = false; }
	}

	void HexagonGame::render(Drawable &mDrawable) { window.draw(mDrawable); }

	void HexagonGame::initFlashEffect()
	{
		flashPolygon.clear();
		flashPolygon.append({{-100.f, -100.f}, Color{255, 255, 255, 0}});
		flashPolygon.append({{getWidth() + 100.f, -100.f}, Color{255, 255, 255, 0}});
		flashPolygon.append({{getWidth() + 100.f, getHeight() + 100.f}, Color{255, 255, 255, 0}});
		flashPolygon.append({{-100.f, getHeight() + 100.f}, Color{255, 255, 255, 0}});
	}

	void HexagonGame::drawText()
	{
		ostringstream s;
		s << "time: " << toStr(status.currentTime).substr(0, 5) << endl;
		if(levelStatus.tutorialMode) s << "tutorial mode" << endl; else if(getOfficial()) s << "official mode" << endl;
		if(getDebug()) s << "debug mode" << endl;
		if(levelStatus.swapEnabled) s << "swap enabled" << endl;
		if(getInvincible()) s << "invincibility on" << endl;
		if(status.scoreInvalid) s << "score invalidated (performance issues)" << endl;
		if(status.hasDied) s << "press r to restart" << endl;
		if(getShowFPS()) s << "FPS: " << window.getFPS() << endl;

		const auto& trackedVariables(levelStatus.trackedVariables);
		if(getShowTrackedVariables() && !trackedVariables.empty())
		{
			s << endl;
			for(const auto& t : trackedVariables)
			{
				if(!lua.doesVariableExist(t.variableName)) continue;
				string var{lua.readVariable<string>(t.variableName)};
				s << t.displayName << ": " << var << endl;
			}
		}

		Vec2f pos{15, 3};
		vector<Vec2f> offsets{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

		Color offsetColor{getColor(1)};
		if(getBlackAndWhite()) offsetColor = Color::Black;
		text.setString(s.str());
		text.setCharacterSize(25 / getZoomFactor());
		text.setOrigin(0, 0);

		text.setColor(offsetColor);
		for(const auto& o : offsets) { text.setPosition(pos + o); render(text); }

		text.setColor(getColorMain());
		text.setPosition(pos);
		render(text);

		if(messageText.getString() == "") return;

		messageText.setOrigin(messageText.getGlobalBounds().width / 2, 0);
		messageText.setColor(offsetColor);
		for(const auto& o : offsets) { messageText.setPosition(Vec2f{getWidth() / 2.f, getHeight() / 6.f} + o); render(messageText); }

		messageText.setColor(getColorMain());
		render(messageText);
	}
}
