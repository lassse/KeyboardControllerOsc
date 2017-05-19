#include "ofApp.h"


void ofApp::setup(){
    oscPort = OSC_PORT;
    oscAddressRoot = OSC_ADDRESS_ROOT;
    
    // Load settings from xml
    ofXml xml;
    xml.load("settings.xml");
    xml.setTo("OSCKeySimulator");
    oscPort = ofToInt(xml.getValue("port"));
    oscAddressRoot = xml.getValue("address");
    oscAddressEnable = xml.getValue("enable");
    oscAddressDisable = xml.getValue("disable");
    
    keyDuration = ofToInt(xml.getValue("key-duration"));
    
    // Key inputs
    xml.setTo("keys");
    totalKeyInputs = xml.getNumChildren();
    xml.setTo("input[0]");
    for (int i = 0; i < totalKeyInputs; i++) {
        int oscId = ofToInt(xml.getValue("osc-id"));
        string keyString = ofToString(xml.getValue("key"));
        int keyCode = keyCodes.convertStringToKeyCode(keyString);
        
        
        KeyInput keyInput;
        keyInput.setup(keyCode, keyString, oscId);
        
        keyInputs.push_back(keyInput);

        
        xml.setToSibling();
    }
    
    
    fadeStep = 255 / stayOnScreenFrames;
    
    receiver.setup(oscPort);
    
}


void ofApp::update(){
    if (!canPressKey) {
        if (ofGetElapsedTimeMillis() - timer >= keyDuration) {
            triggerKeyUp();
        }
    }

    if (receivingOSC && frameCounter < stayOnScreenFrames) {
        frameCounter++;
    }else{
        receivingOSC = false;
    }
    
    
    while (receiver.hasWaitingMessages()) {
        
        receiver.getNextMessage(&oscMessage);
        
            
        if (canPressKey) {
            if (oscMessage.getAddress() == oscAddressEnable) {
                allowVirtualKeystrokes = true;
            }
            
            if (oscMessage.getAddress() == oscAddressDisable) {
                allowVirtualKeystrokes = false;
            }
            
            if (oscMessage.getAddress() == oscAddressRoot) {
                receivingOSC = true;
                frameCounter = 0;
                
                if (allowVirtualKeystrokes) {
//                    oscId = (int) oscMessage.getArgAsFloat();
                    oscId = oscMessage.getArgAsInt(0);
                    int keyCode;
                    for (int i = 0; i < totalKeyInputs; i++) {
                        if (keyInputs[i].oscId == oscId) {
                            keyCode = keyInputs[i].keyCode;
                        }
                    }
                    sendKeyCode(keyCode);
                }
                
            }
            
        }else{
            oscMessage.clear();
        }
    }
}

void ofApp::draw(){
    ofBackground(0, 0, 0);
    
    
    ofSetColor(255, 255, 255);
    ofDrawRectangle(20, 20, 20, 20);

    if (allowVirtualKeystrokes) {
        ofSetColor(0, 150, 0);
        ofDrawRectangle(22, 22, 16, 16);
    }
    
    ofSetColor(255, 255, 255);
    ofDrawBitmapString("Allow keyboard simulation", 50, 34);

    
    ofSetColor(255, 255, 255);
    ofDrawRectangle(20, 50, 20, 20);
    
    if (triggerKeysAsToggles) {
        ofSetColor(0, 150, 0);
        ofDrawRectangle(22, 52, 16, 16);
    }
    
    ofSetColor(255, 255, 255);
    ofDrawBitmapString("Trigger keys as toggles", 50, 63);

    
    
    if (receivingOSC) {
        ofSetColor(0, 150, 0);
    }else{
        ofSetColor(150, 0, 0);
    }
    
    ofDrawCircle(30, 90, 5);
    ofSetColor(255, 255, 255);
    ofDrawBitmapString("Receiving OSC", 50, 94);
    
    
    if (oscId > -1) {
        
        string keyString = "";
        
        for (int i = 0; i < totalKeyInputs; i++) {
            if (keyInputs[i].oscId == oscId) {
                keyString = keyInputs[i].keyString;
            }
        }
        ofSetColor(255 - (fadeStep * frameCounter), 255 - (fadeStep * frameCounter), 255 - (fadeStep * frameCounter));
        ofDrawBitmapString("Keystroke: " + keyString, 20, 130);
    }
}

void ofApp::triggerKeyDown() {
    
    CGEventRef e = CGEventCreateKeyboardEvent (NULL, (CGKeyCode) charKey, true);
    CGEventPost(kCGSessionEventTap, e);
    CFRelease(e);
    
    if (triggerKeysAsToggles) {
        bool found = false;
        
        for (int i = 0; i < pressedKeys.size(); i++) {
            if (pressedKeys[i] == charKey) {
                found = true;
            }
        }
        
        if (found) {
            triggerKeyUp();
        }else{
            pressedKeys.push_back(charKey);
        }
        canPressKey = true;
    }else{
        timer = ofGetElapsedTimeMillis();
        canPressKey = false;
    }
}

void ofApp::triggerKeyUp() {
    CGEventRef a = CGEventCreateKeyboardEvent (NULL, (CGKeyCode) charKey, false);
    CGEventPost(kCGSessionEventTap, a);
    CFRelease(a);
    
    if (triggerKeysAsToggles) {
        int index = -1;
        for (int i = 0; i < pressedKeys.size(); i++) {
            if (pressedKeys[i] == charKey) {
                index = i;
            }
        }
        if (index > -1) {
            pressedKeys.erase(pressedKeys.begin() + index);
            ofLog() << "removed " + ofToString(charKey);
        }

    }
    canPressKey = true;
}

void ofApp::sendKeyCode(int key) {
    if (canPressKey) {
        charKey = (char) key;

        triggerKeyDown();
    }
}

void ofApp::mouseReleased(int x, int y, int button){
    if (x >= 20 && x <= 40 && y >= 20 && y <= 40) {
        allowVirtualKeystrokes = !allowVirtualKeystrokes;
    }
    
    if (x >= 20 && x <= 40 && y >= 50 && y <= 70) {
        triggerKeysAsToggles = !triggerKeysAsToggles;
    }
}
