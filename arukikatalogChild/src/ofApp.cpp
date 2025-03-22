#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	//open the webcam
	grabber.setup(1280, 720);

	sender.init("oF-SPOUT");
}

//--------------------------------------------------------------
void ofApp::update(){
	grabber.update();
	//if (grabber.isFrameNew()) {

		//get the ofPixels and convert to an ofxCvColorImage
		auto pixels = grabber.getPixels();
		//colorImg.setFromPixels(pixels);

		//get the ofCvColorImage as a cv::Mat image to pass to the classifier
		//auto cvMat = cv::cvarrToMat(colorImg.getCvImage());

		//get the restuls as a vector of detected items.
		//each result has an ofRectangle for the bounds, a label which identifies the object and the confidence of the classifier
		//results = classify.classifyFrame(cvMat);
	//}
	
	sender.send(grabber.getTexture());
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofSetColor(255);
	grabber.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

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
