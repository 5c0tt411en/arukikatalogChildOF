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
    fbo.allocate(scaledWidth, scaledHeight, GL_RGB);
    
    oscReceiver.setup(OSC_PORT);  // 受信ポート設定
    triggerState = false;  // 初期値
    scaleValue = 1.0f;     // 初期値
    
//    font.load("ヒラギノ角ゴシック W3.ttc", 50, true, true, true); // フォントのデータを指定する
//    font.setLineHeight(24);       // 行間を指定する
//    font.setLetterSpacing(1.0);   // 文字間を指定する
    
    ofTrueTypeFontSettings settings("ヒラギノ角ゴシック W3.ttc", 50);
//    settings.antialiased = true;
    settings.contours = true;
    settings.addRanges(ofAlphabet::Japanese); // 日本語文字セットを指定
    font.load(settings);
    
    // UTF-8エンコーディングを使用
    ofEnableAlphaBlending();
}

//--------------------------------------------------------------
void ofApp::update(){
	grabber.update();
    
	if (grabber.isFrameNew()) {
        fbo.begin();
        ofClear(0, 0, 0, 255); // FBOのクリア
        // 取得した映像を縮小描画
        grabber.draw(0, 0, scaledWidth, scaledHeight);
        fbo.end();
	}
    
    while (oscReceiver.hasWaitingMessages()) {
        ofxOscMessage msg;
        oscReceiver.getNextMessage(msg);

        if (msg.getAddress() == "/trigger") {
            int val = msg.getArgAsInt(0);  // 整数として受け取る
            triggerState = (val == 1);
            ofLogNotice() << "Received /trigger: " << val;
        }
        else if (msg.getAddress() == "/scale") {
            scaleValue = msg.getArgAsFloat(0);  // 小数値として受け取る
            ofLogNotice() << "Received /scale: " << scaleValue;
        }
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    tick  = ofGetElapsedTimef() - timeStamp;
    
    ofBackground(255);
	ofSetColor(255);
    
    ofPushMatrix();
    // 画面の中心に移動し、90度回転
    ofTranslate(ofGetWidth(), 0);  // 90度回転後の新しい原点
    ofRotateDeg(90);  // 時計回りに90度回転
    // 回転した映像を描画
    grabber.draw(0, 0, ofGetWidth() * 16 / 9, ofGetWidth());
    ofPopMatrix();
    
    
    
    switch (stat) {
        case WAIT:
            if (triggerState) {
                stat = COUNTDOWN;
                timeStamp = ofGetElapsedTimef();
            }
        case COUNTDOWN:
            font.drawString("Hello World!!", 100, 200); // 文字とポジションを指定して描画
        default:
            break;
    }
    
    // Syphon に送信
    syphonServer.publishTexture(&fbo.getTexture());
    
    string info = "FPS: " + ofToString(ofGetFrameRate()) + '\n';
    info += "OSC /trigger : " + ofToString(triggerState) + '\n';
    info += "OSC /scale : " + ofToString(scaleValue) + '\n';
    info += "state : " + ofToString(stat);
    ofDrawBitmapStringHighlight(info, 20, 60);
    
    ofSetColor(255, 0, 0);
    font.drawString("テスト", 100, 500);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if (key == 's') {
            saveFrame();  // 's' で画像保存
    }
}

void ofApp::saveFrame() {
    if (grabber.isFrameNew()) {  // 新しいフレームがあるか確認
        ofPixels pixels = grabber.getPixels();  // ピクセルデータ取得
        
        // 画像を回転するために新しい ofImage を作成
        ofImage rotatedImage;
        rotatedImage.setFromPixels(pixels);
        rotatedImage.rotate90(1);  // 90度回転（1 = 時計回り）

        // タイムスタンプ付きのファイル名を作成
        string fileName = "capture_" + ofGetTimestampString("%Y%m%d_%H%M%S") + ".png";
        
        // 保存
        rotatedImage.save(fileName);
        ofLogNotice() << "Saved image: " << fileName;
    }
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
