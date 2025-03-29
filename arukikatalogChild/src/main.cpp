#include "ofMain.h"
#include "ofApp.h"
#include "ofAppGLFWWindow.h"

//========================================================================
int main( ){
	//Use ofGLFWWindowSettings for more options like multi-monitor fullscreen
    ofGLFWWindowSettings settings;
//	ofGLWindowSettings settings;
//    settings.setGLVersion(4,0);
	settings.setSize(720, 3840);
    settings.setPosition(ofVec2f(0, 0));
//	settings.windowMode = OF_WINDOW; //can also be OF_FULLSCREEN
    settings.decorated = false;   // デコレーションなし
    settings.resizable = false;   // リサイズ不可（必要なら true に）

	auto window = ofCreateWindow(settings);

	ofRunApp(window, make_shared<ofApp>());
	ofRunMainLoop();

}
