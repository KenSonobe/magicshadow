#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"

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
    
    //ライブカメラを使用する際には、カメラ入力を準備
    ofVideoGrabber cam;
    
    ofxCvColorImage srcImg; //オリジナルのカラー映像
    ofxCvGrayscaleImage shadowImg; //グレースケールに変換
    
    int background_color;
    
    int thresh; //閾値
    
    int videoMode; //表示する画像を選択
    
    int width, height;
    
    int dx, dy;
    
    int rotate;
    
    bool jump, jumping, falling, up;
    bool flip;
    bool back;
    bool turnLeft1, turnLeft2;
    
    int time_flip;
    bool flip_timer;
    
    int turn_count;
    int time_turn;
    bool turn_timer;
    
};

