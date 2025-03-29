#pragma once

#include "ofMain.h"
#include "ofxSyphon.h"
#include "ofxOsc.h"

#define OSC_PORT 7000  // 受信ポート

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
        void saveFrame();
		
		ofVideoGrabber grabber;
    
        ofFbo fbo;  // 縮小用のFBO
        ofxSyphonServer syphonServer; // Syphon サーバー
        
        int camWidth = 3840;  // カメラの解像度（幅）
        int camHeight = 2160; // カメラの解像度（高さ）
        int scaledWidth = 640;  // 縮小解像度
        int scaledHeight = 360;
        bool isVertical = false;
    
        float tick, timeStamp;
    
        ofxOscReceiver oscReceiver;  // OSCレシーバー
        bool triggerState;           // /trigger の値
        float scaleValue;            // /scale の値
    
    enum State {
        WAIT,
        COUNTDOWN,
        TAKE_PIC,
    };
    State stat = WAIT;
    ofTrueTypeFont font;
};
