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
        float easeOutCubic(float t, float b, float c, float d);    
		
		ofVideoGrabber grabber;
    
        ofFbo fbo;  // 縮小用のFBO
        ofxSyphonServer syphonServer; // Syphon サーバー
        ofxOscReceiver oscReceiver;  // OSCレシーバー
        ImageSaverThread imageSaver; // 画像保存スレッド
        
        const int camWidth = 3840,  // カメラの解像度（幅）
                  camHeight = 2160, // カメラの解像度（高さ）
                  fboWidth = 640,  // 縮小解像度
                  fboHeight = 360,
                  baseHeight = 680, //655 + 25
                  displayHeight = 1200;
    
        const float countdownConst = 1.0,
                    resetConst = 0.4,
                    scaleToShow = 0.25,
                    mp2mm = 2100, //身長をスケールで割った
                    mm2px = 1.6, //1920 / 1200
                    lenPerDot = 1.0,
                    timeoutSec = 10.0;
    
        int offsetY,
            offsetYEase,
            poseImgY;
        float tick,
              timeStamp,
              resetTick,
              resetTimeStamp,
              scaleValue,
              countdownSec,
              shutterAlpha,
              toSyncAlpha,
              height_mm;
    
        bool triggerState,  // /trigger の値
             showGui,
             isDetected;
    
        enum State {
            WAIT,
            MEASURE,
            HEIGHT_ADJUST,
            POSING,
            COUNTDOWN,
            CHATTERING,
            TAKE_PHOTO,
            END,
        };
        State stat = WAIT;
    
        ofImage waitImg,
                poseImg,
                endImg,
                okImg;  // 画像を保持する変数
    
        ofSoundPlayer shutterSound;  // シャッター音を保持する変数
};
