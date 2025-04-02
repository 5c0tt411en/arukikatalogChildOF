#pragma once

#include "ofMain.h"
#include "ofxSyphon.h"
#include "ofxOsc.h"
#include "ImageSaverThread.hpp"

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
        void exit();  // アプリ終了時の処理を追加
        void saveFrame();
        void drawTextCentered(ofTrueTypeFont &font, const string &text, float x, float y);
		
		ofVideoGrabber grabber;
    
        ofFbo fbo;  // 縮小用のFBO
        ofxSyphonServer syphonServer; // Syphon サーバー
        ofxOscReceiver oscReceiver;  // OSCレシーバー
        ImageSaverThread imageSaver; // 画像保存スレッド
        
        int camWidth = 3840;  // カメラの解像度（幅）
        int camHeight = 2160; // カメラの解像度（高さ）
        int fboWidth = 640;  // 縮小解像度
        int fboHeight = 360;
        bool isVertical = false;
    
        float tick,
              timeStamp,
              resetTick,
              resetTimeStamp,
              scaleValue,
              countdownSec,
              shutterAlpha,
              toSyncAlpha;
    
        const float countdownConst = 5.0,
                    resetConst = 1.0;
    
        bool triggerState;           // /trigger の値
    
        enum State {
            WAIT,
            COUNTDOWN,
            CHATTERING,
            TAKE_PHOTO,
            SYNC,
        };
        State stat = WAIT;
        ofTrueTypeFont font,
                       countdownFont;
};
