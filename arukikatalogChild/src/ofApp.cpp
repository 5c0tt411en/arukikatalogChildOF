#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    // 利用可能なカメラのリストを取得
    vector<ofVideoDevice> devices = grabber.listDevices();
    
    int selectedDeviceID = -1;
    // カメラリストを表示し、「AnkerWork C310 Webcam」を探す
    for(int i = 0; i < devices.size(); i++){
        ofLogNotice() << "Device " << i << ": " << devices[i].deviceName;
        if(devices[i].deviceName.find("AnkerWork C310 Webcam") != string::npos){
            selectedDeviceID = devices[i].id;
        }
    }
    if(selectedDeviceID != -1){
        grabber.setDeviceID(selectedDeviceID);
        ofLogNotice() << "Selected camera: AnkerWork C310 Webcam (ID: " << selectedDeviceID << ")";
    } else {
        ofLogWarning() << "AnkerC310 not found. Using default camera.";
    }

    // カメラの設定
    grabber.setDesiredFrameRate(30);
    grabber.initGrabber(camWidth, camHeight);
    
    // Syphon のセットアップ
    syphonServer.setName("oF-Syphon");

    // 縮小描画用のFBOを作成
    fbo.allocate(fboWidth, fboHeight, GL_RGB);
    
    oscReceiver.setup(OSC_PORT);  // 受信ポート設定
    triggerState = false;  // 初期値
    scaleValue = 1.0f;     // 初期値
    
    // 画像保存スレッドを開始
    imageSaver.start();
    
    waitImg.load("images/waitImg.png");
    poseImg.load("images/poseImg.png");
    okImg.load("images/okImg.png");
    endImg.load("images/endImg.png");
    
    shutterSound.load("sounds/shutter.mp3");
    shutterSound.setVolume(1.0); // 音量設定（0.0〜1.0）
    shutterSound.setMultiPlay(false); // 多重再生を無効化
    
    ofHideCursor();
}

//--------------------------------------------------------------
void ofApp::update(){
    if (stat != TAKE_PHOTO) {
        grabber.update();
    }
    
	if (grabber.isFrameNew()) {
        fbo.begin();
        ofClear(0, 0, 0, 255); // FBOのクリア
        // 取得した映像を縮小描画
        grabber.draw(0, 0, fboWidth, fboHeight);
        fbo.end();
	}
    
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(msg);

        if (msg.getAddress() == "/trigger") {
            int val = msg.getArgAsInt(0);  // 整数として受け取る
            triggerState = (val == 1);
        }
        else if (msg.getAddress() == "/scale") {
            scaleValue = msg.getArgAsFloat(0);  // 小数値として受け取る
            height_mm = mp2mm * scaleValue;
        }
        else if (msg.getAddress() == "/is_detected") {
            int val = msg.getArgAsInt(0);  // 整数として受け取る
            isDetected = (val == 1);
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    tick  = ofGetElapsedTimef() - timeStamp;
    resetTick = ofGetElapsedTimef() - resetTimeStamp;
    
    // 画面の中心座標を取得
    float centerX = ofGetWidth() / 2;
    float centerY = ofGetHeight() / 2;
    
    ofBackground(255);
	ofSetColor(255);
    
    int grabberWidth = scaleToShow * camWidth * 1080 / 360;
    int grabberHeight = scaleToShow * camHeight;
    
    switch (stat) {
        case WAIT:
            ofSetColor(255);
            waitImg.draw(0, 0, ofGetWidth(), ofGetHeight());
            if (isDetected) {
                stat = MEASURE;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case MEASURE:
            offsetY = ofClamp(mm2px * (displayHeight - (height_mm - baseHeight)) - ofGetHeight() / 2, (-ofGetHeight() + grabberHeight) / 2, (ofGetHeight() - grabberHeight) / 2);
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2, grabberWidth, grabberHeight);
            if (isDetected && tick >= 1.0) {
                stat = HEIGHT_ADJUST;
                timeStamp = ofGetElapsedTimef();
            }
            if (!isDetected && tick >= timeoutSec) {
                stat = WAIT;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case HEIGHT_ADJUST:
            offsetYEase = easeOutCubic(tick, 0, 1, 3.0) * offsetY;
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2 + offsetYEase, grabberWidth, grabberHeight);
            if (tick >= 3.0) {
                stat = COUNTDOWN;
                timeStamp = ofGetElapsedTimef();
            }
            if (!isDetected && tick >= timeoutSec) {
                stat = WAIT;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case POSING:
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2 + offsetY, grabberWidth, grabberHeight);
            if (triggerState) {
                stat = COUNTDOWN;
                timeStamp = ofGetElapsedTimef();
            }
            if (!isDetected && tick >= timeoutSec) {
                stat = WAIT;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case COUNTDOWN:
            countdownSec = countdownConst - tick;
            
            if (!triggerState) {
                stat = CHATTERING;
                resetTimeStamp = ofGetElapsedTimef();
            }
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2 + offsetY, grabberWidth, grabberHeight);
            if (countdownSec <= .0) {
                saveFrame();
                shutterSound.play();
                stat = TAKE_PHOTO;
                timeStamp = ofGetElapsedTimef();
                countdownSec = countdownConst;
            }
            break;
        case CHATTERING:
            countdownSec = countdownConst - tick;
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2 + offsetY, grabberWidth, grabberHeight);
            if (triggerState) {
                stat = COUNTDOWN;
                resetTimeStamp = ofGetElapsedTimef();
            }
            
            if (resetTick >= resetConst) {
                stat = POSING;
                timeStamp = ofGetElapsedTimef();
            }
            
            if (countdownSec <= .0) {
                saveFrame();
                shutterSound.play();
                stat = TAKE_PHOTO;
                timeStamp = ofGetElapsedTimef();
                countdownSec = countdownConst;
            }
            break;
        case TAKE_PHOTO:
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2 + offsetY, grabberWidth, grabberHeight);
            okImg.draw(0, (ofGetHeight() - grabberHeight) / 2 + offsetY, ofGetWidth(), okImg.getHeight());
//            toSyncAlpha = ofClamp(128 * (tick - 3.), 0, 255);
//            ofSetColor(255, toSyncAlpha);
//            ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            if (tick >= 5.) {
                stat = END;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case END:
            ofSetColor(255);
            endImg.draw(0, 0, ofGetWidth(), ofGetHeight());
            if (!isDetected && tick >= timeoutSec) {
                stat = WAIT;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        default:
            break;
    }
    
    // Syphon に送信
    ofSetColor(255);
    syphonServer.publishTexture(&fbo.getTexture());
    
    if (showGui) {
        string info = "FPS: " + ofToString(ofGetFrameRate()) + '\n';
        info += "OSC /is_detected : " + ofToString(isDetected) + '\n';
        info += "OSC /trigger : " + ofToString(triggerState) + '\n';
        info += "OSC /scale : " + ofToString(scaleValue) + '\n';
        info += "height(mm) : " + ofToString(height_mm) + '\n';
        info += "offset(px) : " + ofToString(offsetY) + '\n';
        info += "tick : " + ofToString(tick) + '\n';
        info += "state : " + ofToString(stat);
        ofDrawBitmapStringHighlight(info, 20, 60);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == 'g') {
        showGui = !showGui;
    }
}

// リファクタリングした saveFrame 関数
void ofApp::saveFrame() {
    // ピクセルデータを取得
    ofPixels pixels = grabber.getPixels();
    
    // タイムスタンプ付きのファイル名を作成
    string fileName = "outputs/capture_" + ofGetTimestampString("%Y%m%d_%H%M%S") + ".png";
    
    // ピクセルデータをスレッドに渡して、スレッド内で回転・保存
    imageSaver.saveImageWithRotation(pixels, fileName, 0);  // 90度時計回りに回転
    
    ofLogNotice() << "Queued image for rotation and saving: " << fileName;
}

//--------------------------------------------------------------
 void ofApp::exit() {
     // スレッドを正しく終了
     imageSaver.stop();
 }

float ofApp::easeOutCubic(float t, float b, float c, float d) {
    t /= d;
    t--;
    return (c*(t*t*t + 1) + b);
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
