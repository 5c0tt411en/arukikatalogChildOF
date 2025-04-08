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
    
    ofTrueTypeFontSettings settings("fonts/ヒラギノ角ゴシック W3.ttc", 28);
//    settings.antialiased = true;
    settings.contours = true;
    settings.addRanges(ofAlphabet::Japanese);//日本語
    settings.addRange(ofUnicode::Space);//スペース
    settings.addRange(ofUnicode::Latin);//アルファベット等
    settings.addRange(ofUnicode::Latin1Supplement);//記号、アクサン付き文字など
    settings.addRange(ofUnicode::NumberForms);//数字？
    settings.addRange(ofUnicode::Hiragana);//ひらがな
    settings.addRange(ofUnicode::Katakana);//カタカナ
    font.load(settings);
    
    ofTrueTypeFontSettings settings_cd("fonts/ヒラギノ角ゴシック W3.ttc", 100);
    countdownFont.load(settings_cd);
    
    
    // UTF-8エンコーディングを使用
    ofEnableAlphaBlending();
    
    // 画像保存スレッドを開始
    imageSaver.start();
    
    waitImg.load("images/waitImg.jpg");
}

//--------------------------------------------------------------
void ofApp::update(){
    if (stat == WAIT || stat == COUNTDOWN || stat == CHATTERING) {
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
//            ofLogNotice() << "Received /trigger: " << val;
        }
        else if (msg.getAddress() == "/scale") {
            scaleValue = msg.getArgAsFloat(0);  // 小数値として受け取る
//            ofLogNotice() << "Received /scale: " << scaleValue;
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
    
    int grabberWidth = scaleToShow * camWidth;
    int grabberHeight = scaleToShow * camHeight;
//    offsetY = ofGetHeight() - (mp2mm * scaleValue / lenPerDot - baseHeight / lenPerDot);
    
    switch (stat) {
        case WAIT:
            ofSetColor(255, 255, 255, 128 - 128 * cos(tick * 3));
            waitImg.draw(0, 0);
            if (triggerState) {
                stat = COUNTDOWN;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case COUNTDOWN:
            countdownSec = countdownConst - tick;
            
            if (!triggerState) {
                stat = CHATTERING;
                resetTimeStamp = ofGetElapsedTimef();
            }
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2, grabberWidth, grabberHeight);
            if (countdownSec <= .0) {
                saveFrame();
                stat = TAKE_PHOTO;
                timeStamp = ofGetElapsedTimef();
                countdownSec = countdownConst;
            }
            else {
                ofSetColor(0);
                countdownFont.drawString(ofToString(std::ceil(countdownSec)), ofGetWidth() - 200, 200);
            }
            break;
        case CHATTERING:
            countdownSec = countdownConst - tick;
            grabber.draw((ofGetWidth() - grabberWidth) / 2, (ofGetHeight() - grabberHeight) / 2, grabberWidth, grabberHeight);
            if (triggerState) {
                stat = COUNTDOWN;
                resetTimeStamp = ofGetElapsedTimef();
            }
            
            if (resetTick >= resetConst) {
                stat = WAIT;
                timeStamp = ofGetElapsedTimef();
            }
            
            if (countdownSec <= .0) {
                saveFrame();
                stat = TAKE_PHOTO;
                timeStamp = ofGetElapsedTimef();
                countdownSec = countdownConst;
            }
            else {
                ofSetColor(0, 255 * (1 - resetTick));
                countdownFont.drawString(ofToString(std::ceil(countdownSec)), ofGetWidth() - 200, 200);
            }
            break;
        case TAKE_PHOTO:
            shutterAlpha = ofClamp(255 * (1 - tick), 0, 255);
            ofSetColor(255, shutterAlpha);
            ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            toSyncAlpha = ofClamp(75 * (tick - 3.), 0, 150);
            ofSetColor(0, toSyncAlpha);
            ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            if (tick >= 5.) {
                stat = SYNC;
                timeStamp = ofGetElapsedTimef();
            }
            break;
        case SYNC:
            ofSetColor(0, toSyncAlpha);
            ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());
            ofSetColor(255, 128 - 128 * cos(tick * 3));
            drawTextCentered(font, "別の写真を待っています。", centerX, centerY);
            if (tick >= 10.) {
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
        info += "OSC /trigger : " + ofToString(triggerState) + '\n';
        info += "OSC /scale : " + ofToString(scaleValue) + '\n';
        info += "tick : " + ofToString(tick) + '\n';
        info += "state : " + ofToString(stat);
        ofDrawBitmapStringHighlight(info, 20, 60);
    }
}

void ofApp::drawTextCentered(ofTrueTypeFont &font, const string &text, float x, float y) {
    // テキストの寸法を取得
    ofRectangle bounds = font.getStringBoundingBox(text, 0, 0);
    
    // 中央揃えのための座標を計算
    float xCentered = x - bounds.width / 2;
    float yCentered = y + bounds.height / 2; // Y座標はフォントのベースラインが基準なので高さの半分を加算
    
    // 計算した位置にテキストを描画
    font.drawString(text, xCentered, yCentered);
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
